#include "installer.h"
#include "../../kernel/fs/vfs.h"
static int nonempty(const char*s){return s&&*s;}
int mypkg_validate(const struct package_manifest*m){if(!m||!nonempty(m->id)||!nonempty(m->version)||!nonempty(m->entry))return -1;return 0;}
int mypkg_install(const struct package_manifest*m){if(mypkg_validate(m)!=0)return -1;return vfs_create(m->id);}
int mypkg_remove(const char*id){return vfs_delete(id);}
