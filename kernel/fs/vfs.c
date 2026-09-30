#include "vfs.h"
#include "../storage/ata_pio.h"

#define VFS_DISK_LBA 65536u
#define VFS_MAGIC 0x31534656u
#define VFS_VERSION 1u
#define VFS_META_SECTORS 9u
#define VFS_DATA_START (VFS_DISK_LBA + VFS_META_SECTORS)
#define VFS_FILE_SECTORS 8u

typedef struct { uint32_t magic,version,files,generation; uint32_t reserved[124]; } vfs_super;
typedef struct { uint8_t used,reserved0[3]; char name[VFS_NAME_MAX]; uint32_t size,flags,reserved1; } vfs_disk_file;

static struct vfs_file files[VFS_MAX_FILES];
static uint8_t data[VFS_MAX_FILES][VFS_DATA_MAX];
static int persistent;
static uint32_t generation;

static int same(const char*a,const char*b){size_t i=0;while(a[i]&&b[i]&&a[i]==b[i])i++;return a[i]==0&&b[i]==0;}
static int find(const char*n){for(int i=0;i<VFS_MAX_FILES;i++)if(files[i].used&&same(files[i].name,n))return i;return -1;}
static void clear_ram(void){for(int i=0;i<VFS_MAX_FILES;i++){files[i].used=0;files[i].size=0;files[i].flags=0;files[i].name[0]=0;}}

static int write_meta(void){
    uint8_t sector[512]; vfs_super s={0}; s.magic=VFS_MAGIC;s.version=VFS_VERSION;s.generation=++generation;
    for(int i=0;i<VFS_MAX_FILES;i++)if(files[i].used)s.files++;
    for(size_t i=0;i<sizeof(s);i++)sector[i]=((uint8_t*)&s)[i];
    for(size_t i=sizeof(s);i<sizeof(sector);i++)sector[i]=0;
    if(ata_pio_write(VFS_DISK_LBA,sector,1)!=0)return -1;
    uint8_t meta[8*512];for(size_t i=0;i<sizeof(meta);i++)meta[i]=0;
    vfs_disk_file*d=(vfs_disk_file*)meta;
    for(int i=0;i<VFS_MAX_FILES;i++){d[i].used=files[i].used;for(size_t j=0;j<VFS_NAME_MAX;j++)d[i].name[j]=files[i].name[j];d[i].size=files[i].size;d[i].flags=files[i].flags;}
    return ata_pio_write(VFS_DISK_LBA+1,meta,8);
}
static int load_persistent(void){
    uint8_t sector[512];if(ata_pio_read(VFS_DISK_LBA,sector,1)!=0)return -1;
    vfs_super*s=(vfs_super*)sector;if(s->magic!=VFS_MAGIC||s->version!=VFS_VERSION)return -1;
    uint8_t meta[8*512];if(ata_pio_read(VFS_DISK_LBA+1,meta,8)!=0)return -1;
    vfs_disk_file*d=(vfs_disk_file*)meta;clear_ram();generation=s->generation;
    for(int i=0;i<VFS_MAX_FILES;i++){files[i].used=d[i].used?1:0;files[i].size=d[i].size>VFS_DATA_MAX?VFS_DATA_MAX:d[i].size;files[i].flags=d[i].flags;for(size_t j=0;j<VFS_NAME_MAX;j++)files[i].name[j]=d[i].name[j];files[i].name[VFS_NAME_MAX-1]=0;}
    return 0;
}
static int sync_file(int i){
    if(!persistent)return 0;uint32_t sectors=(files[i].size+511u)/512u;if(sectors>VFS_FILE_SECTORS)sectors=VFS_FILE_SECTORS;
    uint8_t sector[512];
    for(uint32_t s=0;s<sectors;s++){for(size_t j=0;j<sizeof(sector);j++)sector[j]=0;uint32_t off=s*512u,left=files[i].size>off?files[i].size-off:0;if(left>512)left=512;for(uint32_t j=0;j<left;j++)sector[j]=data[i][off+j];if(ata_pio_write(VFS_DATA_START+(uint32_t)i*VFS_FILE_SECTORS+s,sector,1)!=0)return -1;}
    return 0;
}
void vfs_init(void){clear_ram();persistent=0;generation=0;ata_pio_init();if(ata_pio_available()&&load_persistent()==0)persistent=1;}
int vfs_create(const char*n){if(!n||!*n||find(n)>=0)return -1;for(int i=0;i<VFS_MAX_FILES;i++)if(!files[i].used){files[i].used=1;files[i].size=0;files[i].flags=0;size_t j=0;for(;j<VFS_NAME_MAX-1&&n[j];j++)files[i].name[j]=n[j];files[i].name[j]=0;if(persistent&&write_meta()!=0){files[i].used=0;return -1;}return 0;}return -1;}
int vfs_write(const char*n,const void*s,uint32_t z){int i=find(n);if(i<0&&vfs_create(n)==0)i=find(n);if(i<0||!s||z>VFS_DATA_MAX)return -1;const uint8_t*p=s;for(uint32_t j=0;j<z;j++)data[i][j]=p[j];files[i].size=z;if(persistent&&(sync_file(i)!=0||write_meta()!=0))return -1;return (int)z;}
int vfs_read(const char*n,void*d,uint32_t cap){int i=find(n);if(i<0||!d)return -1;if(persistent&&files[i].size){uint32_t sectors=(files[i].size+511u)/512u;if(sectors>VFS_FILE_SECTORS)sectors=VFS_FILE_SECTORS;if(ata_pio_read(VFS_DATA_START+(uint32_t)i*VFS_FILE_SECTORS,data[i],sectors)!=0)return -1;}uint32_t z=files[i].size<cap?files[i].size:cap;uint8_t*p=d;for(uint32_t j=0;j<z;j++)p[j]=data[i][j];return (int)z;}
int vfs_delete(const char*n){int i=find(n);if(i<0)return -1;files[i].used=0;files[i].size=0;files[i].name[0]=0;if(persistent&&write_meta()!=0)return -1;return 0;}
int vfs_list(struct vfs_file*out,uint32_t cap){if(!out)return -1;uint32_t z=0;for(int i=0;i<VFS_MAX_FILES&&z<cap;i++)if(files[i].used)out[z++]=files[i];return (int)z;}
int vfs_sync(void){if(!persistent)return 0;for(int i=0;i<VFS_MAX_FILES;i++)if(files[i].used&&sync_file(i)!=0)return -1;return write_meta();}
int vfs_persistent_available(void){return persistent;}
