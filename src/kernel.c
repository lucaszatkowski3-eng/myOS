#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "../kernel/fs/vfs.h"
#include "../kernel/memory/memory.h"
#include "../kernel/process/process.h"
#include "../system/mypkg/installer.h"

__attribute__((used,section(".requests"))) static volatile uint64_t base_revision[] = LIMINE_BASE_REVISION(3);
__attribute__((used,section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request={.id=LIMINE_FRAMEBUFFER_REQUEST_ID,.revision=0};
__attribute__((used,section(".requests_start"))) static volatile uint64_t requests_start[]=LIMINE_REQUESTS_START_MARKER;
__attribute__((used,section(".requests_end"))) static volatile uint64_t requests_end[]=LIMINE_REQUESTS_END_MARKER;

static uint32_t *fb; static uint64_t width,height,pitch;
static void px(int x,int y,uint32_t c){if(x<0||y<0||(uint64_t)x>=width||(uint64_t)y>=height)return;fb[(uint64_t)y*(pitch/4)+(uint64_t)x]=c;}
static void rect(int x,int y,int w,int h,uint32_t c){if(w<=0||h<=0)return;for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)px(xx,yy,c);}
static void line(int x,int y,int w,int h,uint32_t c){rect(x,y,w,h,c);}
static const uint8_t font[37][5]={
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
{0x32,0x49,0x49,0x49,0x3e}};
static int fi(char c){if(c>='A'&&c<='Z')return c-'A';if(c>='a'&&c<='z')return c-'a';if(c>='0'&&c<='9')return 27+c-'0';return 26;}
static void txt(int x,int y,const char*s,uint32_t c,int z){for(;*s;s++){int n=fi(*s);for(int a=0;a<5;a++)for(int b=0;b<7;b++)if(font[n][a]&(1u<<b))rect(x+a*z,y+b*z,z,z,c);x+=6*z;}}
static void clear(uint32_t c){rect(0,0,(int)width,(int)height,c);}
static void logo(int x,int y,int s){
    /* Original myOS mark: a split M/cube symbol, deliberately independent of Windows branding. */
    uint32_t a=0x007bdcff,b=0x004a6fff;
    rect(x,y,s,s,0x00101820);
    for(int i=0;i<s/3;i++){rect(x+s/8+i,y+s/6+i,s/6,s/6,a);rect(x+s*5/8-i,y+s/6+i,s/6,s/6,a);}
    rect(x+s/3,y+s/3,s/3,s/6,b);
    rect(x+s/4,y+s*2/3,s/2,s/6,a);
}
static char browser_url[192]="myos://home"; static size_t browser_len=10;
static char transfer_name[64]=""; static size_t transfer_len=0;
static uint8_t inb(uint16_t p){uint8_t v;__asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p));return v;}
static int mouse_x=640,mouse_y=360; static uint8_t mouse_cycle,mouse_packet[3];
static void mouse_wait(int t){uint32_t n=100000;if(t==0){while(n--&&!(inb(0x64)&1));}else{while(n--&&(inb(0x64)&2));}}
static void mouse_cmd(uint8_t v){mouse_wait(1);__asm__ volatile("outb %0,%1"::"a"(v),"Nd"((uint16_t)0x64));}
static void mouse_data(uint8_t v){mouse_wait(1);__asm__ volatile("outb %0,%1"::"a"(v),"Nd"((uint16_t)0x60));}
static void mouse_init(void){mouse_cmd(0xA8);mouse_cmd(0x20);mouse_wait(0);uint8_t s=inb(0x60);s|=2;mouse_cmd(0x60);mouse_data(s);mouse_cmd(0xD4);mouse_data(0xF6);mouse_wait(0);inb(0x60);mouse_cmd(0xD4);mouse_data(0xF4);mouse_wait(0);inb(0x60);}

static char key(uint8_t s){
switch(s){case 1:return 27;case 2:return'1';case 3:return'2';case 4:return'3';case 5:return'4';case 6:return'5';case 7:return'6';case 8:return'7';case 9:return'8';case 10:return'9';case 11:return'0';
case 12:return'-';case 13:return'=';case 14:return'\b';case 16:return'Q';case 17:return'W';case 18:return'E';case 19:return'R';case 20:return'T';case 21:return'Y';case 22:return'U';case 23:return'I';case 24:return'O';case 25:return'P';
case 28:return'\n';case 30:return'A';case 31:return'S';case 32:return'D';case 33:return'F';case 34:return'G';case 35:return'H';case 36:return'J';case 37:return'K';case 38:return'L';case 44:return'Z';case 45:return'X';case 46:return'C';case 47:return'V';case 48:return'B';case 49:return'N';case 50:return'M';case 51:return',';case 52:return'.';case 53:return'/';case 57:return' ';default:return 0;}}
static char keyboard(void){if(!(inb(0x64)&1))return 0;uint8_t s=inb(0x60);if(s&0x80)return 0;return key(s);}

enum app_id{DESKTOP,STORE,WRITE,SHEETS,CALC,SLIDES,TERM,FILES,SETTINGS,BROWSER,TRANSFER};
static enum app_id app=DESKTOP; static int menu=0,store_sel=0,slide=1;
static char input[256];static size_t input_len=0;static char document[1024];static size_t document_len=0;
struct pkg{const char*name;const char*id;int installed;};
static struct pkg pkgs[]={
{"myWrite","org.myos.mywrite",0},{"mySheets","org.myos.mysheets",0},{"myCalculator","org.myos.mycalculator",0},
{"mySlides","org.myos.myslides",0},{"Terminal","org.myos.terminal",1},{"File Manager","org.myos.files",1},
{"Snake","org.myos.games.snake",0},{"Mines","org.myos.games.mines",0},{"Tetris","org.myos.games.tetris",0}};

static void header(const char*n){rect(0,0,(int)width,54,0x00151e28);rect(0,53,(int)width,1,0x003c566d);logo(18,7,38);txt(68,17,"myOS",0x007bdcff,2);txt(150,19,n,0x00e7eef5,1);}
static void bottom(void){rect(0,(int)height-54,(int)width,54,0x00101820);rect(0,(int)height-55,(int)width,1,0x003c566d);txt(24,(int)height-36,"MENU",0x007bdcff,2);txt(125,(int)height-34,"1 STORE  2 WRITE  3 SHEETS  4 CALC  5 SLIDES  6 TERM  7 FILES  8 SETTINGS  9 WEB  0 SHARE",0x00c8d5df,1);}
static void icon(int x,int y,const char*n,uint32_t c){rect(x,y,92,70,0x00202d38);rect(x+12,y+10,42,34,c);txt(x+10,y+49,n,0x00dbe5ec,1);}
static void desktop(void){
clear(0x000d151d);rect(0,0,(int)width,(int)height-55,0x00121d27);
txt(34,28,"WORKSPACE",0x00dbe5ec,2);logo((int)width-92,10,34);
icon(34,75,"WRITE",0x0071a8ff);icon(146,75,"SHEETS",0x0055c78a);icon(258,75,"CALC",0x00f1a34b);icon(370,75,"SLIDES",0x00db6b74);
icon(34,165,"STORE",0x009c7cf5);icon(146,165,"FILES",0x004e9fd1);icon(258,165,"TERM",0x007f8b98);icon(370,165,"SETTINGS",0x00b17bdc);icon(482,165,"BROWSER",0x007bdcff);icon(594,165,"SHARE",0x0055c78a);
rect(510,75,360,205,0x001b2935);txt(532,97,"SYSTEM STATUS",0x00ffffff,2);
txt(532,137,"myOS 64-bit",0x007bdcff,1);txt(532,163,"DESKTOP READY",0x008bd8aa,1);txt(532,189,"PERSISTENT STORAGE",vfs_persistent_available()?0x008bd8aa:0x00e7b06a,1);
txt(532,215,vfs_persistent_available()?"DISK ONLINE":"RAM MODE",0x00cbd6df,1);txt(532,241,"APP RUNTIME READY",0x008bd8aa,1);txt(532,260,"NETWORK DRIVER: FOUNDATION",0x00cbd6df,1);
rect(510,300,360,125,0x001b2935);txt(532,322,"QUICK HELP",0x00ffffff,2);txt(532,356,"1-6 OPEN APPS",0x00cbd6df,1);txt(532,380,"ESC RETURN / MENU",0x00cbd6df,1);txt(532,404,"TYPE IN ACTIVE APPS",0x00cbd6df,1);bottom();
}
static void store(void){
clear(0x000d151d);header("APP HUB");txt(28,78,"MY STORE",0x00ffffff,2);
for(int i=0;i<9;i++){int y=110+i*43;rect(28,y,650,39,i==store_sel?0x002b4558:0x001b2935);txt(42,y+10,pkgs[i].name,0x00ffffff,1);txt(220,y+10,pkgs[i].installed?"INSTALLED":"FREE",pkgs[i].installed?0x008bd8aa:0x00e7b06a,1);}
txt(28,(int)height-82,"1-9 SELECT   I INSTALL   U REMOVE   ENTER LAUNCH",0x00b9c8d4,1);bottom();
}
static void write_app(void){
clear(0x00eef1f4);header("myWrite");rect(42,78,(int)width-84,(int)height-155,0x00ffffff);txt(62,98,"DOCUMENT",0x00243a49,2);
int x=62,y=140;for(size_t i=0;i<document_len&&y<(size_t)height-90;i++){char s[2]={document[i],0};if(document[i]=='\n'){x=62;y+=22;continue;}txt(x,y,s,0x001c2a34,2);x+=12;if(x>(int)width-105){x=62;y+=22;}}
txt(44,(int)height-78,"TYPE   BACKSPACE   C CLEAR   S SAVE",0x005f7280,1);bottom();
}
static long calc_value(void){long total=0,n=0;char op='+';for(size_t i=0;i<=input_len;i++){char c=i<input_len?input[i]:0;if(c>='0'&&c<='9')n=n*10+c-'0';else if(c=='+'||c=='-'||c=='*'||c=='/'||!c){if(op=='+')total+=n;else if(op=='-')total-=n;else if(op=='*')total*=n;else if(op=='/'&&n)total/=n;n=0;op=c;}}return total;}
static void number(long n,char*o){char t[32];int p=0,q=0;if(n==0){o[0]='0';o[1]=0;return;}if(n<0){o[q++]='-';n=-n;}while(n&&p<31){t[p++]=(char)('0'+n%10);n/=10;}while(p)o[q++]=t[--p];o[q]=0;}
static void sheets(void){clear(0x000e171e);header("mySheets");txt(45,86,"SPREADSHEET",0x00ffffff,2);for(int i=0;i<5;i++)for(int j=0;j<4;j++){rect(45+j*150,125+i*42,148,40,0x001b2a34);if(i==0)txt(58+j*150,138,"CELL",0x007bdcff,1);}txt(45,360,"mySheets is the document table workspace.",0x00cbd6df,1);bottom();}
static void calc(void){clear(0x000e171e);header("myCalculator");rect(60,82,650,110,0x001b2a34);txt(82,102,input,0x00ffffff,2);char r[32];number(calc_value(),r);txt(82,142,r,0x007bdcff,3);txt(60,230,"NUMBERS  +  -  *  /    C CLEAR",0x00cbd6df,1);bottom();}
static void slides(void){clear(0x00151b23);header("mySlides");rect(70,82,(int)width-140,(int)height-160,0x00f8fafb);if(slide==1){txt(130,160,"myOS",0x00263e50,5);txt(135,240,"A DIFFERENT DESKTOP",0x003c89b7,2);}else if(slide==2){txt(130,150,"APPS",0x00263e50,4);txt(135,215,"myWrite",0x003c89b7,2);txt(135,255,"mySheets",0x003c89b7,2);txt(135,295,"myCalculator",0x003c89b7,2);}else{txt(130,160,"MY STORE",0x00263e50,4);txt(135,230,"FREE COMMUNITY APPS",0x003c89b7,2);}txt(72,(int)height-78,"A PREVIOUS   D NEXT",0x005f7280,1);bottom();}
static void terminal(void){clear(0x00080e10);header("Terminal");txt(28,86,"myOS shell",0x007bdcff,2);txt(28,120,"help  apps  files  clear  date  version",0x00b9c8d4,1);txt(28,170,input,0x00ffffff,1);txt(28,200,">",0x007bdcff,2);bottom();}
static void files(void){clear(0x000d151d);header("Files");txt(28,82,"/home",0x00ffffff,2);struct vfs_file f[16];int n=vfs_list(f,16);if(n<0)n=0;for(int i=0;i<n;i++){int y=125+i*30;txt(42,y,f[i].name,0x00dbe5ec,1);char z[16];number(f[i].size,z);txt(330,y,z,0x008fa6b6,1);}txt(28,620,"Files stored by myVFS survive reboot on the disk image.",0x008fa6b6,1);bottom();}
static void browser(void){
clear(0x00eef1f4);header("myBrowser");
rect(34,76,(int)width-68,48,0x00ffffff);txt(48,91,browser_url,0x00243a49,2);
txt(36,150,"myBrowser",0x00243a49,4);
txt(36,205,"A native myOS web interface.",0x003c89b7,2);
txt(36,245,"Enter a URL in the address bar.",0x005f7280,1);
txt(36,275,"Networking is currently in driver/stack development.",0x005f7280,1);
txt(36,315,"Built-in: myos://home   myos://files   myos://about",0x005f7280,1);
txt(36,370,"ENTER NAVIGATE   BACKSPACE EDIT",0x005f7280,1);bottom();
}
static void transfer(void){
clear(0x000d151d);header("myShare");
txt(34,88,"PHONE → myOS",0x00ffffff,3);
txt(34,140,"Simple file exchange workspace",0x007bdcff,2);
rect(34,180,(int)width-68,52,0x001b2935);
txt(48,196,transfer_name,0x00ffffff,2);
txt(34,260,"Files placed in the transfer area are stored in myVFS.",0x00cbd6df,1);
txt(34,292,"On a booted PC, USB/network transport still needs the",0x00cbd6df,1);
txt(34,314,"remaining USB or network driver. The UI is ready for it.",0x00cbd6df,1);
txt(34,365,"TYPING: choose a filename   ENTER: create empty file",0x008fa6b6,1);
txt(34,395,"FILES: open File Manager to inspect transferred files.",0x008fa6b6,1);
bottom();
}
static void settings(void){clear(0x101922);header("Settings");txt(32,90,"MYOS SETTINGS",0x00ffffff,2);txt(32,135,"Display",0x007bdcff,1);txt(32,165,"Keyboard",0x007bdcff,1);txt(32,195,"Storage",0x007bdcff,1);txt(32,225,vfs_persistent_available()?"Persistent storage: ONLINE":"Persistent storage: RAM MODE",0x00cbd6df,1);txt(32,255,"Network services are being expanded in the next system layer.",0x00cbd6df,1);bottom();}
static void redraw(void){if(app==DESKTOP)desktop();else if(app==STORE)store();else if(app==WRITE)write_app();else if(app==SHEETS)sheets();else if(app==CALC)calc();else if(app==SLIDES)slides();else if(app==TERM)terminal();else if(app==FILES)files();else if(app==BROWSER)browser();else if(app==TRANSFER)transfer();else settings();}
static void back(void){app=DESKTOP;menu=0;input_len=0;redraw();}
static void command(void){
input[input_len]=0;
if(input_len==5&&input[0]=='c'&&input[1]=='l'&&input[2]=='e'&&input[3]=='a'&&input[4]=='r')input_len=0;
else if(input_len==4&&input[0]=='a'&&input[1]=='p'&&input[2]=='p'&&input[3]=='s'){}
else if(input_len==5&&input[0]=='f'&&input[1]=='i'&&input[2]=='l'&&input[3]=='e'&&input[4]=='s')app=FILES;
else if(input_len==7&&input[0]=='v'&&input[1]=='e'&&input[2]=='r'&&input[3]=='s'&&input[4]=='i'&&input[5]=='o'&&input[6]=='n'){}
else if(input_len==4&&input[0]=='h'&&input[1]=='e'&&input[2]=='l'&&input[3]=='p'){}
else input_len=0;redraw();
}
static void mouse_poll(void){
 while(inb(0x64)&1){
  uint8_t b=inb(0x60);
  if(mouse_cycle==0 && !(b&8)) continue;
  mouse_packet[mouse_cycle++]=b;
  if(mouse_cycle==3){
   int dx=(int8_t)mouse_packet[1],dy=(int8_t)mouse_packet[2];
   mouse_x+=dx;mouse_y-=dy;
   if(mouse_x<0)mouse_x=0;if(mouse_y<0)mouse_y=0;
   if(mouse_x>=(int)width)mouse_x=(int)width-1;if(mouse_y>=(int)height)mouse_y=(int)height-1;
   if(mouse_packet[0]&1 && app==DESKTOP){
    if(mouse_y>=75&&mouse_y<145){if(mouse_x<126)app=WRITE;else if(mouse_x<238)app=SHEETS;else if(mouse_x<350)app=CALC;else if(mouse_x<462)app=SLIDES;}
    else if(mouse_y>=165&&mouse_y<235){if(mouse_x<126)app=STORE;else if(mouse_x<238)app=FILES;else if(mouse_x<350)app=TERM;else if(mouse_x<462)app=SETTINGS;else if(mouse_x<574)app=BROWSER;else if(mouse_x<686)app=TRANSFER;}
    else if(mouse_y>=(int)height-70){if(mouse_x<110)app=STORE;else if(mouse_x<200)app=WRITE;else if(mouse_x<290)app=SHEETS;else if(mouse_x<380)app=CALC;else if(mouse_x<470)app=SLIDES;else if(mouse_x<650)app=TERM;}
    redraw();
   }
   mouse_cycle=0;
  }
 }
}
static void handle(char c){
if(!c)return;
if(c==27){back();return;}
if(app==DESKTOP){if(c=='1')app=STORE;else if(c=='2')app=WRITE;else if(c=='3')app=SHEETS;else if(c=='4')app=CALC;else if(c=='5')app=SLIDES;else if(c=='6')app=TERM;else if(c=='7')app=FILES;else if(c=='8')app=SETTINGS;else if(c=='9')app=BROWSER;else if(c=='0')app=TRANSFER;redraw();return;}
if(app==STORE){if(c>='1'&&c<='9'){store_sel=c-'1';redraw();return;}if(c=='I'){pkgs[store_sel].installed=1;struct package_manifest m={pkgs[store_sel].id,"0.1.0","bin/app"};mypkg_install(&m);redraw();return;}if(c=='U'){pkgs[store_sel].installed=0;redraw();return;}if(c=='\n'){app=store_sel==0?WRITE:store_sel==1?SHEETS:store_sel==2?CALC:store_sel==3?SLIDES:store_sel==4?TERM:store_sel==5?FILES:DESKTOP;redraw();return;}return;}
if(app==WRITE){if(c=='C'){document_len=0;redraw();return;}if(c=='S'){vfs_write("home/document.txt",document,(uint32_t)document_len);redraw();return;}if(c=='\b'){if(document_len)document_len--;redraw();return;}if(c>=32&&c<=126){if(document_len<sizeof(document)-1)document[document_len++]=c;redraw();return;}}
if(app==CALC||app==SHEETS){if(c=='C'){input_len=0;redraw();return;}if(c=='\b'){if(input_len)input_len--;redraw();return;}if((c>='0'&&c<='9')||c=='+'||c=='-'||c=='*'||c=='/'){if(input_len<sizeof(input)-1)input[input_len++]=c;redraw();return;}}
if(app==SLIDES){if(c=='A'&&slide>1)slide--;if(c=='D'&&slide<3)slide++;redraw();return;}
if(app==TERM){if(c=='\b'){if(input_len)input_len--;redraw();return;}if(c=='\n'){command();return;}if(c>=32&&c<=126&&input_len<sizeof(input)-1)input[input_len++]=c;redraw();return;}
if(app==BROWSER){if(c=='\b'){if(browser_len)browser_url[--browser_len]=0;redraw();return;}if(c=='\n'){redraw();return;}if(c>=32&&c<=126&&browser_len<sizeof(browser_url)-1){browser_url[browser_len++]=c;browser_url[browser_len]=0;redraw();return;}}
if(app==TRANSFER){if(c=='\b'){if(transfer_len)transfer_name[--transfer_len]=0;redraw();return;}if(c=='\n'){if(transfer_len){vfs_create(transfer_name);transfer_len=0;transfer_name[0]=0;}redraw();return;}if(c>=32&&c<=126&&transfer_len<sizeof(transfer_name)-1){transfer_name[transfer_len++]=c;transfer_name[transfer_len]=0;redraw();return;}}
}
static void desktop_task(void*arg){(void)arg;}
void kmain(void){
if(!LIMINE_BASE_REVISION_SUPPORTED(base_revision))for(;;)__asm__ volatile("hlt");
if(!framebuffer_request.response||framebuffer_request.response->framebuffer_count<1)for(;;)__asm__ volatile("hlt");
struct limine_framebuffer*f=framebuffer_request.response->framebuffers[0];fb=(uint32_t*)f->address;width=f->width;height=f->height;pitch=f->pitch;
memory_init();process_init();process_create("desktop",desktop_task,0);vfs_init();mouse_init();redraw();
for(;;){handle(keyboard());mouse_poll();scheduler_tick();__asm__ volatile("hlt");}}
