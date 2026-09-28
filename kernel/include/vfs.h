#pragma once
#include <stdint.h>
typedef enum { VFS_FILE, VFS_DIRECTORY } vfs_type_t;
typedef struct vfs_node { char name[128]; vfs_type_t type; uint64_t size; uint32_t mode; void *private_data; } vfs_node_t;
int vfs_init(void); int vfs_mount_root(void); int vfs_mkdir(const char *path); int vfs_create(const char *path);
