#include "ipp.h"
int ipp_parse_header(const unsigned char*b,size_t n,struct ipp_request*r){if(!b||!r||n<8)return-1;r->major=b[0];r->minor=b[1];r->operation=(uint16_t)(((uint16_t)b[2]<<8)|b[3]);r->request_id=((uint32_t)b[4]<<24)|((uint32_t)b[5]<<16)|((uint32_t)b[6]<<8)|b[7];if(r->major!=1&&r->major!=2)return-2;return 0;}
const char*ipp_operation_name(uint16_t op){switch(op){case IPP_OP_PRINT_JOB:return"Print-Job";case IPP_OP_VALIDATE_JOB:return"Validate-Job";case IPP_OP_GET_PRINTER_ATTRIBUTES:return"Get-Printer-Attributes";default:return"Unknown";}}
