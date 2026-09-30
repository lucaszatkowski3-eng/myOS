#include "vfs.h"
static struct vfs_file files[VFS_MAX_FILES];
static uint8_t data[VFS_MAX_FILES][VFS_DATA_MAX];
static int same(const char *a,const char *b){size_t i=0;while(a[i]&&b[i]&&a[i]==b[i])i++;return a[i]==0&&b[i]==0;}
static int find(const char *n){for(int i=0;i<VFS_MAX_FILES;i++)if(files[i].used&&same(files[i].name,n))return i;return -1;}
void vfs_init(void){for(int i=0;i<VFS_MAX_FILES;i++){files[i].used=0;files[i].size=0;files[i].name[0]=0;}}
int vfs_create(const char *n){if(!n||!*n||find(n)>=0)return -1;for(int i=0;i<VFS_MAX_FILES;i++)if(!files[i].used){files[i].used=1;size_t j=0;for(;j<VFS_NAME_MAX-1&&n[j];j++)files[i].name[j]=n[j];files[i].name[j]=0;return 0;}return -1;}
int vfs_write(const char*n,const void*s,uint32_t z){int i=find(n);if(i<0&&vfs_create(n)==0)i=find(n);if(i<0||!s||z>VFS_DATA_MAX)return -1;const uint8_t*p=s;for(uint32_t j=0;j<z;j++)data[i][j]=p[j];files[i].size=z;return (int)z;}
int vfs_read(const char*n,void*d,uint32_t cap){int i=find(n);if(i<0||!d)return -1;uint32_t z=files[i].size<cap?files[i].size:cap;uint8_t*p=d;for(uint32_t j=0;j<z;j++)p[j]=data[i][j];return (int)z;}
int vfs_delete(const char*n){int i=find(n);if(i<0)return -1;files[i].used=0;files[i].size=0;return 0;}
int vfs_list(struct vfs_file*out,uint32_t cap){uint32_t z=0;for(int i=0;i<VFS_MAX_FILES&&z<cap;i++)if(files[i].used)out[z++]=files[i];return (int)z;}
int vfs_sync(void){return 0;}
int vfs_persistent_available(void){return 0;}
