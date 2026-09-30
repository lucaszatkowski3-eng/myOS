#include "elf_loader.h"
static uint16_t r16(const uint8_t*p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
static uint64_t r64(const uint8_t*p){uint64_t v=0;for(int i=0;i<8;i++)v|=(uint64_t)p[i]<<(i*8);return v;}
int elf64_inspect(const void *image,uint64_t size,struct elf64_info*out){
 if(!image||!out||size<64)return -1;
 const uint8_t*p=(const uint8_t*)image;
 if(p[0]!=0x7f||p[1]!='E'||p[2]!='L'||p[3]!='F'||p[4]!=2||p[5]!=1)return -1;
 if(r16(p+18)!=0x3e)return -1;
 uint64_t phoff=r64(p+32); uint16_t phentsz=r16(p+54),phnum=r16(p+56);
 if(phentsz<56||phnum>128||phoff>(size-1)||phoff+(uint64_t)phentsz*phnum>size)return -1;
 out->valid=1;out->machine=r16(p+18);out->type=r16(p+16);out->entry=r64(p+24);out->phnum=phnum;return 0;
}
