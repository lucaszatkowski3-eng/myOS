#include "pe.h"
static uint16_t rd16(const uint8_t*p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
static uint32_t rd32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
int pe_inspect(const void*img,uint32_t size,struct pe_info*out){
 if(!img||!out||size<64)return -1; const uint8_t*p=(const uint8_t*)img;
 if(p[0]!='M'||p[1]!='Z')return -1; uint32_t off=rd32(p+0x3c);
 if(off>size-24||p[off]!='P'||p[off+1]!='E'||p[off+2]!=0||p[off+3]!=0)return -1;
 out->machine=rd16(p+off+4); out->sections=rd16(p+off+6);
 if(off+24>size)return -1; uint16_t opt=rd16(p+off+20);
 if(off+24+opt>size)return -1;
 out->is_64bit=rd16(p+off+24)==0x20b; out->valid=1; return 0;
}
