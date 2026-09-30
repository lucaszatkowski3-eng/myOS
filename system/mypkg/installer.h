#pragma once
#include <stdint.h>
struct package_manifest { const char *id; const char *version; const char *entry; };
int mypkg_validate(const struct package_manifest *m);
int mypkg_install(const struct package_manifest *m);
int mypkg_remove(const char *id);
