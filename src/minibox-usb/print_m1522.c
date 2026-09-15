#include "libusb_m1522.h"
#include "../minibox-ipp/print_job.h"

static int m1522_sink(void *ctx,const unsigned char *buf,int len,int timeout_ms){
    return m1522_bulk_write((struct m1522_handle *)ctx,buf,len,timeout_ms);
}

int m1522_print_document(const unsigned char *doc,size_t len){
    struct m1522_handle h;
    int r;
    if(!doc||!len)return-1;
    r=m1522_open(&h);
    if(r)return r;
    r=minibox_print_document(m1522_sink,&h,doc,len,16384,5000);
    m1522_close(&h);
    return r;
}
