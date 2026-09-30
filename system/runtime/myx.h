#pragma once
#include <stdint.h>
#define MYX_MAGIC 0x3158594Du
#define MYX_VERSION 1u
enum myx_opcode { MYX_END=0, MYX_CLEAR=1, MYX_RECT=2, MYX_TEXT=3, MYX_WAIT_KEY=4 };
struct myx_host {
    void (*clear)(uint32_t color);
    void (*rect)(int x,int y,int w,int h,uint32_t color);
    void (*text)(int x,int y,const char *text,uint32_t color,int scale);
    char (*key)(void);
};
int myx_validate(const void *image,uint32_t size);
int myx_run(const void *image,uint32_t size,const struct myx_host *host);
