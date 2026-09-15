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
static volatile sig_atomic_t stop; static void on_signal(int s){(void)s;stop=1;}
static int send_all(int f,const char*p,size_t n){while(n){ssize_t w=send(f,p,n,0);if(w<0){if(errno==EINTR)continue;return-1;}p+=w;n-=(size_t)w;}return 0;}
static void out(int f,int code,const char*type,const char*body){char h[512];size_t n=strlen(body);int m=snprintf(h,sizeof h,"HTTP/1.1 %d OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",code,type,n);if(m>0){send_all(f,h,(size_t)m);send_all(f,body,n);}}
static void serve(int f){char b[8192],m[16],p[256];ssize_t n=recv(f,b,sizeof b-1,0);if(n<=0)return;b[n]=0;if(sscanf(b,"%15s %255s",m,p)!=2){out(f,400,"text/plain","bad request\n");return;}if(!strcmp(m,"GET")&&!strcmp(p,"/health")){out(f,200,"text/plain","minibox-scand ok\n");return;}if(!strcmp(m,"GET")&&!strcmp(p,"/eSCL/ScannerCapabilities")){out(f,200,"text/xml","<?xml version=\"1.0\"?><scan:ScannerCapabilities xmlns:scan=\"http://schemas.hp.com/imaging/escl/2011/05/03\"><scan:MakeAndModel>HP LaserJet M1522n @ MiniBox</scan:MakeAndModel><scan:Platen/><scan:Adf/></scan:ScannerCapabilities>\n");return;}if(!strcmp(m,"GET")&&!strcmp(p,"/eSCL/ScannerStatus")){out(f,200,"text/xml","<?xml version=\"1.0\"?><scan:ScannerStatus xmlns:scan=\"http://schemas.hp.com/imaging/escl/2011/05/03\"><scan:State>Idle</scan:State></scan:ScannerStatus>\n");return;}if(!strcmp(m,"POST")&&!strcmp(p,"/eSCL/ScanJobs")){out(f,501,"text/plain","eSCL transport alive; USB scan backend pending\n");return;}out(f,404,"text/plain","not found\n");}
int main(int argc,char**argv){int port=argc>1?atoi(argv[1]):8080,s=socket(AF_INET,SOCK_STREAM,0),one=1;if(s<0){perror("socket");return 1;}setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one);struct sockaddr_in a;memset(&a,0,sizeof a);a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_ANY);a.sin_port=htons((unsigned short)port);if(bind(s,(struct sockaddr*)&a,sizeof a)||listen(s,8)){perror("bind/listen");return 1;}signal(SIGINT,on_signal);signal(SIGTERM,on_signal);fprintf(stderr,"minibox-scand: listening on %d\n",port);while(!stop){int c=accept(s,NULL,NULL);if(c<0){if(errno==EINTR)continue;break;}serve(c);close(c);}close(s);return 0;}
