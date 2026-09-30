#include "myx.h"
struct myx_header { uint32_t magic; uint16_t version; uint16_t flags; uint32_t code_size; uint32_t data_size; };
static uint16_t rd16(const uint8_t *p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
static uint32_t rd32(const uint8_t *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static int header(const uint8_t *p,uint32_t size){
    if(size<16 || rd32(p)!=MYX_MAGIC || rd16(p+4)!=MYX_VERSION)return -1;
    uint32_t cs=rd32(p+8),ds=rd32(p+12);
    return (cs>size-16 || ds>size-16-cs) ? -1 : 0;
}
int myx_validate(const void *image,uint32_t size){return header((const uint8_t*)image,size);}
int myx_run(const void *image,uint32_t size,const struct myx_host *host){
    if(!host||myx_validate(image,size)!=0)return -1;
    const uint8_t *p=(const uint8_t*)image;uint32_t code=rd32(p+8),data=rd32(p+12);
    const uint8_t *pc=p+16,*end=pc+code;
    while(pc<end){
        uint8_t op=*pc++;
        switch(op){
            case MYX_END:return 0;
            case MYX_CLEAR:
                if(pc+4>end||!host->clear)return -1;host->clear(rd32(pc));pc+=4;break;
            case MYX_RECT:
                if(pc+20>end||!host->rect)return -1;
                host->rect((int)rd32(pc),(int)rd32(pc+4),(int)rd32(pc+8),(int)rd32(pc+12),rd32(pc+16));pc+=20;break;
            case MYX_TEXT:{
                if(pc+17>end||!host->text)return -1;
                int x=(int)rd32(pc),y=(int)rd32(pc+4),scale=(int)rd32(pc+8);uint32_t color=rd32(pc+12);uint8_t len=pc[16];pc+=17;
                if(pc+len>end||len>=128)return -1;
                char s[128];for(uint32_t i=0;i<len;i++)s[i]=(char)pc[i];s[len]=0;host->text(x,y,s,color,scale);pc+=len;break;
            }
            case MYX_WAIT_KEY:if(!host->key)return -1;(void)host->key();break;
            default:return -1;
        }
    }
    (void)data;return 0;
}
