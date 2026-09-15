#include "../src/minibox-ipp/ipp.h"
#include <assert.h>
#include <string.h>
int main(void){const unsigned char q[]={2,0,0,0x0b,0,0,0,7};struct ipp_request r;assert(ipp_parse_header(q,sizeof q,&r)==0);assert(r.major==2&&r.minor==0&&r.operation==IPP_OP_GET_PRINTER_ATTRIBUTES&&r.request_id==7);assert(!strcmp(ipp_operation_name(IPP_OP_PRINT_JOB),"Print-Job"));assert(ipp_parse_header(q,7,&r)==-1);return 0;}
