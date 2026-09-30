#include "ata_pio.h"

#define ATA_DATA 0x1F0
#define ATA_SECCOUNT 0x1F2
#define ATA_LBA0 0x1F3
#define ATA_LBA1 0x1F4
#define ATA_LBA2 0x1F5
#define ATA_DRIVE 0x1F6
#define ATA_STATUS 0x1F7
#define ATA_CONTROL 0x3F6
#define ATA_SR_ERR 0x01
#define ATA_SR_DRQ 0x08
#define ATA_SR_DF  0x20
#define ATA_SR_BSY 0x80

static int present;

static inline void outb(uint16_t p, uint8_t v){ __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p)); }
static inline uint8_t inb(uint16_t p){ uint8_t v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline void insw(uint16_t p, uint16_t *d, uint32_t n){ __asm__ volatile("rep insw":"+D"(d),"+c"(n):"d"(p):"memory"); }
static inline void outsw(uint16_t p, const uint16_t *s, uint32_t n){ __asm__ volatile("rep outsw":"+S"(s),"+c"(n):"d"(p)); }
static void io_wait(void){ (void)inb(ATA_CONTROL); (void)inb(ATA_CONTROL); (void)inb(ATA_CONTROL); (void)inb(ATA_CONTROL); }

static int wait_not_busy(void){
    for(uint32_t i=0;i<1000000;i++){
        uint8_t s=inb(ATA_STATUS);
        if(!(s&ATA_SR_BSY)) return (s&ATA_SR_ERR)||(s&ATA_SR_DF) ? -1 : 0;
    }
    return -1;
}
static int wait_drq(void){
    for(uint32_t i=0;i<1000000;i++){
        uint8_t s=inb(ATA_STATUS);
        if(s&ATA_SR_ERR || s&ATA_SR_DF) return -1;
        if(!(s&ATA_SR_BSY) && (s&ATA_SR_DRQ)) return 0;
    }
    return -1;
}

void ata_pio_init(void){
    present=0;
    outb(ATA_CONTROL,0x02);
    outb(ATA_DRIVE,0xA0);
    io_wait();
    uint8_t status=inb(ATA_STATUS);
    if(status==0xFF || status==0) return;
    if(wait_not_busy()!=0) return;
    outb(ATA_SECCOUNT,0);
    outb(ATA_LBA0,0);
    outb(ATA_LBA1,0);
    outb(ATA_LBA2,0);
    outb(ATA_DRIVE,0xE0);
    outb(ATA_STATUS,0xEC);
    status=inb(ATA_STATUS);
    if(status==0 || status==0xFF) return;
    if(wait_not_busy()!=0) return;
    uint8_t mid=inb(ATA_LBA1), hi=inb(ATA_LBA2);
    if(mid!=0 || hi!=0) return;
    if(wait_drq()!=0) return;
    uint16_t identify[256];
    insw(ATA_DATA,identify,256);
    present=1;
}
int ata_pio_available(void){ return present; }

static int transfer(uint32_t lba, uint8_t *buf, int write){
    if(!present || lba>0x0FFFFFFFu || !buf) return -1;
    if(wait_not_busy()!=0) return -1;
    outb(ATA_DRIVE,0xE0|((lba>>24)&0x0F));
    outb(ATA_SECCOUNT,1);
    outb(ATA_LBA0,(uint8_t)lba);
    outb(ATA_LBA1,(uint8_t)(lba>>8));
    outb(ATA_LBA2,(uint8_t)(lba>>16));
    outb(ATA_STATUS,write?0x30:0x20);
    if(wait_drq()!=0) return -1;
    if(write) outsw(ATA_DATA,(const uint16_t*)buf,256);
    else insw(ATA_DATA,(uint16_t*)buf,256);
    io_wait();
    if(write){ outb(ATA_STATUS,0xE7); if(wait_not_busy()!=0) return -1; }
    return 0;
}
int ata_pio_read(uint32_t lba, void *buffer, uint32_t sectors){
    if(!buffer || !sectors) return -1;
    uint8_t *p=(uint8_t*)buffer;
    for(uint32_t i=0;i<sectors;i++) if(transfer(lba+i,p+i*512,0)!=0) return -1;
    return 0;
}
int ata_pio_write(uint32_t lba, const void *buffer, uint32_t sectors){
    if(!buffer || !sectors) return -1;
    const uint8_t *p=(const uint8_t*)buffer;
    for(uint32_t i=0;i<sectors;i++) if(transfer(lba+i,(uint8_t*)(p+i*512),1)!=0) return -1;
    return 0;
}
