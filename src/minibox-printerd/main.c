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
#include "../minibox-ipp/ipp.h"
#ifndef MINIBOX_TEST_PRINT_SINK
#include "../minibox-usb/print_m1522.h"
#endif

static volatile sig_atomic_t stop;
static void on_signal(int sig){(void)sig;stop=1;}
static int send_all(int fd,const void*vp,size_t n){const unsigned char*p=vp;while(n){ssize_t w=send(fd,p,n,0);if(w<0){if(errno==EINTR)continue;return-1;}p+=w;n-=(size_t)w;}return 0;}
static void reply_data(int fd,int code,const char*reason,const char*type,const void*body,size_t n){char h[512];int m=snprintf(h,sizeof h,"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",code,reason,type,n);if(m>0){(void)send_all(fd,h,(size_t)m);if(n)(void)send_all(fd,body,n);}}
static void reply_text(int fd,int code,const char*reason,const char*body){reply_data(fd,code,reason,"text/plain",body,strlen(body));}
static const unsigned char*find_body(const unsigned char*b,size_t n,size_t*body_n){size_t i;for(i=0;i+3<n;i++)if(b[i]=='\r'&&b[i+1]=='\n'&&b[i+2]=='\r'&&b[i+3]=='\n'){*body_n=n-i-4;return b+i+4;}*body_n=0;return NULL;}

static int print_document(const unsigned char *doc,size_t n){
#ifdef MINIBOX_TEST_PRINT_SINK
    const char *path=getenv("MINIBOX_TEST_PRINT_FILE");FILE*f;if(!path)return-90;f=fopen(path,"wb");if(!f)return-91;if(fwrite(doc,1,n,f)!=n){fclose(f);return-92;}return fclose(f)?-93:0;
#else
    return m1522_print_document(doc,n);
#endif
}

static void serve_ipp(int fd,const unsigned char*body,size_t body_n){struct ipp_request r;unsigned char out[64];size_t out_n;uint16_t status=0x0000;if(ipp_parse_header(body,body_n,&r)){reply_text(fd,400,"Bad Request","invalid IPP header\n");return;}switch(r.operation){case IPP_OP_GET_PRINTER_ATTRIBUTES:case IPP_OP_VALIDATE_JOB:status=0x0000;break;case IPP_OP_PRINT_JOB:{size_t off=0;if(ipp_document_offset(body,body_n,&off)||off>=body_n)status=0x0400;else status=print_document(body+off,body_n-off)?0x0507:0x0000;break;}default:status=0x0501;break;}out_n=ipp_build_status(out,sizeof out,&r,status);reply_data(fd,200,"OK","application/ipp",out,out_n);}
static void serve(int fd){unsigned char b[65536];ssize_t n=recv(fd,b,sizeof b,0);char method[16],path[256];size_t body_n=0;const unsigned char*body;if(n<=0)return;if(sscanf((const char*)b,"%15s %255s",method,path)!=2){reply_text(fd,400,"Bad Request","bad request\n");return;}if(!strcmp(method,"GET")&&!strcmp(path,"/health")){reply_text(fd,200,"OK","minibox-printerd ok\n");return;}if(strcmp(path,"/ipp/print")){reply_text(fd,404,"Not Found","not found\n");return;}if(strcmp(method,"POST")){reply_text(fd,405,"Method Not Allowed","POST required\n");return;}body=find_body(b,(size_t)n,&body_n);if(!body){reply_text(fd,400,"Bad Request","missing HTTP body\n");return;}serve_ipp(fd,body,body_n);}
int main(int argc,char**argv){int port=argc>1?atoi(argv[1]):631;int s=socket(AF_INET,SOCK_STREAM,0);if(s<0){perror("socket");return 1;}int one=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one);struct sockaddr_in a;memset(&a,0,sizeof a);a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_ANY);a.sin_port=htons((unsigned short)port);if(bind(s,(struct sockaddr*)&a,sizeof a)||listen(s,8)){perror("bind/listen");close(s);return 1;}signal(SIGINT,on_signal);signal(SIGTERM,on_signal);fprintf(stderr,"minibox-printerd: listening on %d\n",port);while(!stop){int c=accept(s,NULL,NULL);if(c<0){if(errno==EINTR)continue;perror("accept");break;}serve(c);close(c);}close(s);return 0;}
