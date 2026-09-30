#include "installer.h"
#include "../../kernel/fs/vfs.h"

static int nonempty(const char*s){return s&&*s;}
static int build_name(char*out,const char*prefix,const char*id){
    size_t p=0;while(prefix[p]&&p<31){out[p]=prefix[p];p++;}
    size_t i=0;while(id[i]&&p<47){out[p++]=id[i++];}
    out[p]=0;return id[i]? -1:0;
}
int mypkg_validate(const struct package_manifest*m){
    if(!m||!nonempty(m->id)||!nonempty(m->version)||!nonempty(m->entry))return -1;
    return 0;
}
int mypkg_install(const struct package_manifest*m){
    if(mypkg_validate(m)!=0)return -1;
    char marker[48],meta[48],text[192];
    if(build_name(marker,"app:",m->id)!=0||build_name(meta,"meta:",m->id)!=0)return -1;
    int n=0;const char *a=m->id,*b=m->version,*c=m->entry;
    while(*a&&n<(int)sizeof(text)-4)text[n++]=*a++;
    text[n++]='|';while(*b&&n<(int)sizeof(text)-3)text[n++]=*b++;
    text[n++]='|';while(*c&&n<(int)sizeof(text)-2)text[n++]=*c++;
    text[n]=0;
    if(vfs_create(marker)!=0 && vfs_write(marker,text,(uint32_t)n)<0)return -1;
    if(vfs_write(meta,text,(uint32_t)n)<0)return -1;
    return vfs_sync();
}
int mypkg_remove(const char*id){
    if(!nonempty(id))return -1;
    char marker[48],meta[48];
    if(build_name(marker,"app:",id)!=0||build_name(meta,"meta:",id)!=0)return -1;
    vfs_delete(marker);vfs_delete(meta);return vfs_sync();
}
