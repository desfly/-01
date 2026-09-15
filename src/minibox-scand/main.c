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
#include "../minibox-escl/escl.h"
#include "scan_session.h"
static volatile sig_atomic_t stop; static void on_signal(int s){(void)s;stop=1;}
static unsigned next_job=1; static struct minibox_scan_session scan;
static int send_all(int f,const char*p,size_t n){while(n){ssize_t w=send(f,p,n,0);if(w<0){if(errno==EINTR)continue;return-1;}p+=w;n-=(size_t)w;}return 0;}
static void outx(int f,int code,const char*reason,const char*type,const char*extra,const char*body){char h[768];size_t n=strlen(body);int m=snprintf(h,sizeof h,"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n%sConnection: close\r\n\r\n",code,reason,type,n,extra?extra:"");if(m>0){send_all(f,h,(size_t)m);send_all(f,body,n);}}
static const char *reason_for(int code){switch(code){case 200:return "OK";case 201:return "Created";case 400:return "Bad Request";case 404:return "Not Found";case 409:return "Conflict";case 503:return "Service Unavailable";default:return "Error";}}
static void out(int f,int code,const char*type,const char*body){outx(f,code,reason_for(code),type,NULL,body);}
static const char*body_of(char*b,size_t n,size_t*len){size_t i;for(i=0;i+3<n;i++)if(b[i]=='\r'&&b[i+1]=='\n'&&b[i+2]=='\r'&&b[i+3]=='\n'){*len=n-i-4;return b+i+4;}*len=0;return NULL;}
static int next_document_id(const char *p,unsigned *id){char tail;return sscanf(p,"/eSCL/ScanJobs/%u/NextDocument%c",id,&tail)==1?0:-1;}
static void serve(int f){char b[16384],m[16],p[256];ssize_t n=recv(f,b,sizeof b-1,0);if(n<=0)return;b[n]=0;if(sscanf(b,"%15s %255s",m,p)!=2){out(f,400,"text/plain","bad request\n");return;}if(!strcmp(m,"GET")&&!strcmp(p,"/health")){out(f,200,"text/plain","minibox-scand ok\n");return;}if(!strcmp(m,"GET")&&!strcmp(p,"/eSCL/ScannerCapabilities")){out(f,200,"text/xml","<?xml version=\"1.0\"?><scan:ScannerCapabilities xmlns:scan=\"http://schemas.hp.com/imaging/escl/2011/05/03\"><scan:MakeAndModel>HP LaserJet M1522n @ MiniBox</scan:MakeAndModel><scan:Platen/><scan:Adf/></scan:ScannerCapabilities>\n");return;}if(!strcmp(m,"GET")&&!strcmp(p,"/eSCL/ScannerStatus")){out(f,200,"text/xml",minibox_scan_session_busy(&scan)?"<?xml version=\"1.0\"?><scan:ScannerStatus xmlns:scan=\"http://schemas.hp.com/imaging/escl/2011/05/03\"><scan:State>Processing</scan:State></scan:ScannerStatus>\n":"<?xml version=\"1.0\"?><scan:ScannerStatus xmlns:scan=\"http://schemas.hp.com/imaging/escl/2011/05/03\"><scan:State>Idle</scan:State></scan:ScannerStatus>\n");return;}if(!strcmp(m,"POST")&&!strcmp(p,"/eSCL/ScanJobs")){size_t z=0;const char*x=body_of(b,(size_t)n,&z);char extra[256];struct escl_job settings;unsigned id;if(minibox_scan_session_busy(&scan)){out(f,409,"text/plain","scan job already active\n");return;}if(!x||escl_parse_scan_settings(x,z,&settings)){out(f,400,"text/plain","invalid scan settings\n");return;}id=next_job++;if(minibox_scan_session_create(&scan,id,&settings)){out(f,503,"text/plain","cannot create scan job\n");return;}snprintf(extra,sizeof extra,"Location: /eSCL/ScanJobs/%u\r\n",id);outx(f,201,"Created","text/plain",extra,"");return;}if(!strcmp(m,"GET")){unsigned id;if(next_document_id(p,&id)==0){if(minibox_scan_session_begin_page(&scan,id)){out(f,404,"text/plain","no matching scan job\n");return;}/* USB SOAPHT image acquisition is wired here next. Preserve the job on backend failure so clients can retry. */minibox_scan_session_fail(&scan);out(f,503,"text/plain","M1522 SOAPHT image acquisition not available yet\n");return;}}out(f,404,"text/plain","not found\n");}
int main(int argc,char**argv){int port=argc>1?atoi(argv[1]):8080,s=socket(AF_INET,SOCK_STREAM,0),one=1;if(s<0){perror("socket");return 1;}setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one);struct sockaddr_in a;memset(&a,0,sizeof a);a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_ANY);a.sin_port=htons((unsigned short)port);if(bind(s,(struct sockaddr*)&a,sizeof a)||listen(s,8)){perror("bind/listen");return 1;}signal(SIGINT,on_signal);signal(SIGTERM,on_signal);fprintf(stderr,"minibox-scand: listening on %d\n",port);while(!stop){int c=accept(s,NULL,NULL);if(c<0){if(errno==EINTR)continue;break;}serve(c);close(c);}close(s);return 0;}
