#pragma once
#include <stdint.h>
#include <stddef.h>
#define VFS_MAX_FILES 64
#define VFS_NAME_MAX 48
#define VFS_DATA_MAX 4096
struct vfs_file { uint8_t used; char name[VFS_NAME_MAX]; uint32_t size; uint32_t flags; };
void vfs_init(void);
int vfs_create(const char *name);
int vfs_write(const char *name,const void *data,uint32_t size);
int vfs_read(const char *name,void *data,uint32_t capacity);
int vfs_delete(const char *name);
int vfs_list(struct vfs_file *out,uint32_t capacity);
int vfs_sync(void);
int vfs_persistent_available(void);
