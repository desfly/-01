#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t stop;
static void on_signal(int sig) { (void)sig; stop = 1; }

static int send_all(int fd, const char *p, size_t n) {
    while (n) { ssize_t w = send(fd, p, n, 0); if (w < 0) { if (errno == EINTR) continue; return -1; } p += w; n -= (size_t)w; }
    return 0;
}

static void reply(int fd, int code, const char *reason, const char *body) {
    char h[512]; size_t n = strlen(body);
    int m = snprintf(h, sizeof h, "HTTP/1.1 %d %s\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", code, reason, n);
    if (m > 0) { (void)send_all(fd, h, (size_t)m); (void)send_all(fd, body, n); }
}

static void serve(int fd) {
    char b[8192]; ssize_t n = recv(fd, b, sizeof b - 1, 0); if (n <= 0) return; b[n] = 0;
    char method[16], path[256]; if (sscanf(b, "%15s %255s", method, path) != 2) { reply(fd,400,"Bad Request","bad request\n"); return; }
    if (!strcmp(method,"GET") && !strcmp(path,"/health")) { reply(fd,200,"OK","minibox-printerd ok\n"); return; }
    if (!strcmp(path,"/ipp/print")) {
        if (!strcmp(method,"POST")) reply(fd,501,"Not Implemented","IPP transport alive; IPP parser and libusb backend pending\n");
        else reply(fd,405,"Method Not Allowed","POST required\n");
        return;
    }
    reply(fd,404,"Not Found","not found\n");
}

int main(int argc, char **argv) {
    int port = argc > 1 ? atoi(argv[1]) : 631; int s = socket(AF_INET, SOCK_STREAM, 0); if (s < 0) { perror("socket"); return 1; }
    int one=1; setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one);
    struct sockaddr_in a; memset(&a,0,sizeof a); a.sin_family=AF_INET; a.sin_addr.s_addr=htonl(INADDR_ANY); a.sin_port=htons((unsigned short)port);
    if (bind(s,(struct sockaddr*)&a,sizeof a) || listen(s,8)) { perror("bind/listen"); close(s); return 1; }
    signal(SIGINT,on_signal); signal(SIGTERM,on_signal); fprintf(stderr,"minibox-printerd: listening on %d\n",port);
    while (!stop) { int c=accept(s,NULL,NULL); if(c<0){if(errno==EINTR)continue; perror("accept"); break;} serve(c); close(c); }
    close(s); return 0;
}
