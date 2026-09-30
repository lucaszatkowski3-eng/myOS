#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "../kernel/fs/vfs.h"
#include "../kernel/memory/memory.h"
#include "../kernel/process/process.h"

__attribute__((used, section(".requests")))
static volatile uint64_t base_revision[] = LIMINE_BASE_REVISION(3);
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0
};
__attribute__((used, section(".requests_start")))
static volatile uint64_t requests_start[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end")))
static volatile uint64_t requests_end[] = LIMINE_REQUESTS_END_MARKER;

static uint32_t *fb;
static uint64_t width, height, pitch;

static void putpixel(int x, int y, uint32_t c) {
    if (x < 0 || y < 0 || (uint64_t)x >= width || (uint64_t)y >= height) return;
    fb[(uint64_t)y * (pitch / 4) + (uint64_t)x] = c;
}
static void rect(int x, int y, int w, int h, uint32_t c) {
    if (w <= 0 || h <= 0) return;
    for (int yy=y; yy<y+h; ++yy) for (int xx=x; xx<x+w; ++xx) putpixel(xx,yy,c);
}
static const uint8_t font[37][5] = {
 {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},{0x3e,0x41,0x41,0x41,0x22},
 {0x7f,0x41,0x41,0x22,0x1c},{0x7f,0x49,0x49,0x49,0x41},{0x7f,0x09,0x09,0x09,0x01},
 {0x3e,0x41,0x49,0x49,0x7a},{0x7f,0x08,0x08,0x08,0x7f},{0,0x41,0x7f,0x41,0},
 {0x20,0x40,0x41,0x3f,0x01},{0x7f,0x08,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
 {0x7f,0x02,0x04,0x02,0x7f},{0x7f,0x04,0x08,0x10,0x7f},{0x3e,0x41,0x41,0x41,0x3e},
 {0x7f,0x09,0x09,0x09,0x06},{0x3e,0x41,0x51,0x21,0x5e},{0x7f,0x09,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7f,0x01,0x01},{0x3f,0x40,0x40,0x40,0x3f},
 {0x1f,0x20,0x40,0x20,0x1f},{0x7f,0x20,0x18,0x20,0x7f},{0x63,0x14,0x08,0x14,0x63},
 {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},{0,0,0,0,0},
 {0x3e,0x45,0x49,0x51,0x3e},{0x00,0x21,0x7f,0x01,0x00},{0x21,0x43,0x45,0x49,0x31},
 {0x22,0x41,0x49,0x49,0x36},{0x0c,0x14,0x24,0x7f,0x04},{0x79,0x49,0x49,0x49,0x46},
 {0x3e,0x49,0x49,0x49,0x26},{0x40,0x47,0x48,0x50,0x60},{0x36,0x49,0x49,0x49,0x36},
 {0x32,0x49,0x49,0x49,0x3e}
};
static int idx(char c) {
    if (c>='A' && c<='Z') return c-'A';
    if (c>='a' && c<='z') return c-'a';
    if (c>='0' && c<='9') return 27+(c-'0');
    return 26;
}
static void text(int x,int y,const char *s,uint32_t c,int scale) {
    for (;*s;++s) {
        int n=idx(*s);
        for(int col=0;col<5;++col) for(int row=0;row<7;++row)
            if(font[n][col]&(1u<<row)) rect(x+col*scale,y+row*scale,scale,scale,c);
        x+=6*scale;
    }
}
static uint8_t inb(uint16_t p) {
    uint8_t v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p)); return v;
}
static char keymap(uint8_t s) {
    switch(s) {
        case 2:return '1'; case 3:return '2'; case 4:return '3'; case 5:return '4';
        case 6:return '5'; case 7:return '6'; case 8:return '7'; case 9:return '8';
        case 10:return '9'; case 11:return '0'; case 12:return '-'; case 13:return '=';
        case 16:return 'Q'; case 17:return 'W'; case 18:return 'E'; case 19:return 'R';
        case 20:return 'T'; case 21:return 'Y'; case 22:return 'U'; case 23:return 'I';
        case 24:return 'O'; case 25:return 'P'; case 30:return 'A'; case 31:return 'S';
        case 32:return 'D'; case 33:return 'F'; case 34:return 'G'; case 35:return 'H';
        case 36:return 'J'; case 37:return 'K'; case 38:return 'L'; case 44:return 'Z';
        case 45:return 'X'; case 46:return 'C'; case 47:return 'V'; case 48:return 'B';
        case 49:return 'N'; case 50:return 'M'; case 51:return ','; case 52:return '.';
        case 53:return '/'; case 57:return ' '; case 28:return 'E'; case 14:return 'I';
        default:return 0;
    }
}
static char keyboard(void) {
    if(!(inb(0x64)&1)) return 0;
    uint8_t s=inb(0x60); if(s&0x80) return 0; return keymap(s);
}

enum app_id { APP_DESKTOP,APP_STORE,APP_WRITE,APP_SHEETS,APP_CALCULATOR,APP_SLIDES,APP_TERMINAL };
static enum app_id app=APP_DESKTOP;
struct package { const char *id,*name,*version; uint8_t installed; };
static struct package packages[] = {
 {"org.myos.mywrite","myWrite","0.1.0",0},
 {"org.myos.mycalc","mySheets","0.1.0",0},
 {"org.myos.myslides","mySlides","0.1.0",0},
 {"org.myos.terminal","Terminal","0.1.0",1},
 {"org.myos.files","File Manager","0.1.0",1},
 {"org.myos.games.snake","Snake","0.1.0",0},
 {"org.myos.games.mines","Mines","0.1.0",0},
 {"org.myos.games.tetris","Tetris","0.1.0",0}
};
static int selected_pkg=0, slide=1;
static void desktop_task(void *arg){ (void)arg; }

static char document[768]; static size_t document_len;
static char expression[96]; static size_t expression_len;
static void icon(int x,int y,uint32_t bg,const char *label){
    rect(x,y,72,58,bg); rect(x+10,y+8,52,36,0x00ffffff); text(x+8,y+70,label,0x00ffffff,1);
}
static void desktop_icons(void){
    icon(28,72,0x003b8fe8,"MYWRITE"); icon(28,174,0x0028a85b,"MYSHEETS");
    icon(28,276,0x00f0782c,"MYSLIDES"); icon(28,378,0x003c8dff,"MYSTORE");
    icon(28,480,0x003d8fe8,"FILES"); icon(28,582,0x006d7884,"SETTINGS");
}

static void clear(uint32_t c){rect(0,0,(int)width,(int)height,c);}
static void titlebar(const char *n){
    rect(0,0,(int)width,44,0x00242d38); text(20,13,"MYOS",0x00ffffff,2);
    text(120,13,n,0x00d9e8ff,2); text((int)width-130,13,"ESC DESKTOP",0x009fb0c0,1);
}
static void desktop(void){
    clear(0x00131a22); rect(0,0,(int)width,(int)height-52,0x001e2935); desktop_icons();
    rect(70,70,430,270,0x000f151b); rect(70,70,430,38,0x002b3947);
    text(88,82,"MYOS DESKTOP",0x00ffffff,2); text(92,135,"WELCOME TO MYOS",0x00e8edf2,2);
    text(92,175,"1 STORE",0x0079c2ff,2); text(92,210,"2 MYWRITE",0x0079c2ff,2);
    text(92,245,"3 MYSHEETS",0x0079c2ff,2); text(92,280,"4 MYSLIDES",0x0079c2ff,2);
    text(92,315,"5 TERMINAL",0x0079c2ff,2);
    rect(540,70,360,270,0x0010181f); rect(540,70,360,38,0x002b3947);
    text(558,82,"SYSTEM",0x00ffffff,2); text(560,135,"MYOS 64",0x0094d4ff,2);
    text(560,175,"PACKAGE MANAGER READY",0x00bfe6c7,1);
    text(560,200,"MYSTORE READY",0x00bfe6c7,1);
    text(560,225,"NETWORK: E1000 TARGET",0x00f2cf88,1);
    text(560,250,"WIFI DRIVERS: NEXT",0x00f2cf88,1);
    rect(0,(int)height-52,(int)width,52,0x0010171e);
    text(18,(int)height-34,"START",0x00ffffff,2); text(120,(int)height-34,"1 STORE",0x00d9e2ea,1);
    text(205,(int)height-34,"2 WRITE",0x00d9e2ea,1); text(285,(int)height-34,"3 SHEETS",0x00d9e2ea,1);
}
static void store(void){
    clear(0x00131a22); titlebar("MYSTORE"); text(24,70,"FIRST PARTY APPLICATIONS",0x00ffffff,2);
    for(int i=0;i<8;i++){int y=95+i*55; rect(24,y,600,54,i==selected_pkg?0x0031485b:0x001d2832);
        text(42,y+10,packages[i].name,0x00ffffff,2); text(220,y+12,packages[i].version,0x009db0bf,1);
        text(330,y+12,packages[i].installed?"INSTALLED":"AVAILABLE",packages[i].installed?0x00a9e6bb:0x00f0ca7b,1);}
    text(24,(int)height-62,"1-8 SELECT   I INSTALL",0x00aebdca,1);
}
static void write_app(void){
    clear(0x00f1f3f5); titlebar("MYWRITE"); rect(40,62,(int)width-80,(int)height-130,0x00ffffff);
    text(58,82,"DOCUMENT",0x00202b35,2); int x=58,y=120;
    for(size_t i=0;i<document_len && y<height-90;i++){if(document[i]==' '){x+=12;continue;}
        char s[2]={document[i],0}; text(x,y,s,0x001d2730,2); x+=12; if(x>(int)width-90){x=58;y+=20;}}
    text(42,(int)height-58,"TYPE TO WRITE   C CLEAR   ESC BACK",0x00c8d4df,1);
}
static long calculate(void){
    long total=0,current=0; char op='+';
    for(size_t i=0;i<=expression_len;i++){char c=i<expression_len?expression[i]:0;
        if(c>='0'&&c<='9') current=current*10+(c-'0');
        else if(c=='+'||c=='-'||c=='*'||c=='/'||c==0){
            if(op=='+')total+=current; else if(op=='-')total-=current; else if(op=='*')total*=current;
            else if(op=='/'&&current)total/=current; current=0; op=c;}}
    return total;
}
static void numstr(long n,char *o){
    char t[24];int p=0,q=0;if(n==0){o[0]='0';o[1]=0;return;}if(n<0){o[q++]='-';n=-n;}
    while(n&&p<23){t[p++]=(char)('0'+n%10);n/=10;}while(p)o[q++]=t[--p];o[q]=0;
}
static void calc(void){
    clear(0x00151d25);titlebar("MYSHEETS");rect(60,75,720,120,0x00202d38);
    text(84,92,"FORMULA",0x008ea5b7,1);text(84,125,expression,0x00ffffff,3);
    text(60,225,"USE NUMBERS + - * /",0x00d6e0e8,1);char r[32];numstr(calculate(),r);
    text(60,270,"RESULT",0x008ea5b7,1);text(60,300,r,0x0079c2ff,4);
}
static void calculator(void){ clear(0x00151d25); titlebar("MYCALCULATOR"); rect(80,75,620,100,0x00202d38); text(105,92,"CALCULATOR",0x008ea5b7,1); text(105,125,expression,0x00ffffff,3); char r[32]; numstr(calculate(),r); text(105,220,"RESULT",0x008ea5b7,1); text(105,250,r,0x0079c2ff,4); text(80,340,"NUMBERS + - * /   C CLEAR",0x00d6e0e8,1); }\nstatic void slides(void){
    clear(0x00202531);titlebar("MYSLIDES");rect(70,70,(int)width-140,(int)height-160,0x00ffffff);
    if(slide==1){text(120,150,"MYOS",0x001d2d3a,5);text(125,235,"THE NEW DESKTOP",0x003a8bc1,2);}
    else if(slide==2){text(120,140,"MYOS APPS",0x001d2d3a,4);text(125,215,"MYWRITE",0x003a8bc1,2);text(125,250,"MYSHEETS",0x003a8bc1,2);text(125,285,"MYSLIDES",0x003a8bc1,2);}
    else{text(120,150,"MYSTORE",0x001d2d3a,4);text(125,225,"INSTALL APPS",0x003a8bc1,2);}
}
static void terminal(void){
    clear(0x000b0f12);titlebar("TERMINAL");text(30,70,"MYOS SHELL",0x0079c2ff,2);
    text(30,110,"MYSTORE  MYWRITE  MYSHEETS  MYSLIDES",0x00d6e0e8,1);
    text(30,145,"NETWORK E1000 TARGET / WIFI NEXT",0x00a9e6bb,1);
}
static void redraw(void){
    if(app==APP_DESKTOP)desktop(); else if(app==APP_STORE)store(); else if(app==APP_WRITE)write_app();
    else if(app==APP_SHEETS)calc(); else if(app==APP_CALCULATOR)calculator(); else if(app==APP_SLIDES)slides(); else terminal();
}
static void handle(char c){
    if(!c)return;
    if(app==APP_DESKTOP){if(c=='1')app=APP_STORE;else if(c=='2')app=APP_WRITE;else if(c=='3')app=APP_SHEETS;else if(c=='4')app=APP_CALCULATOR;else if(c=='5')app=APP_SLIDES;else if(c=='6')app=APP_TERMINAL;redraw();return;}
    if(c=='I'&&app==APP_STORE){packages[selected_pkg].installed=1;redraw();return;}
    if(c>='1'&&c<='8'){if(app==APP_STORE)selected_pkg=c-'1';redraw();return;}
    if(c=='C'&&app==APP_WRITE){document_len=0;redraw();return;}
    if(c=='C'&&(app==APP_SHEETS||app==APP_CALCULATOR)){expression_len=0;redraw();return;}
    if(app==APP_WRITE&&c>=32&&c<=126){if(document_len<sizeof(document)-1)document[document_len++]=c;redraw();return;}
    if(app==APP_SHEETS&&((c>='0'&&c<='9')||c=='+'||c=='-'||c=='*'||c=='/')){if(expression_len<sizeof(expression)-1)expression[expression_len++]=c;redraw();return;}
    if(app==APP_SLIDES){if(c=='A'&&slide>1)--slide;if(c=='D'&&slide<3)++slide;redraw();return;}
}
void kmain(void){
    if(!LIMINE_BASE_REVISION_SUPPORTED(base_revision))for(;;)__asm__ volatile("hlt");
    if(!framebuffer_request.response||framebuffer_request.response->framebuffer_count<1)for(;;)__asm__ volatile("hlt");
    struct limine_framebuffer *f=framebuffer_request.response->framebuffers[0];
    fb=(uint32_t*)f->address;width=f->width;height=f->height;pitch=f->pitch;memory_init();process_init();process_create("desktop",desktop_task,0);vfs_init();redraw();
    for(;;){handle(keyboard());scheduler_tick();__asm__ volatile("hlt");}
}
