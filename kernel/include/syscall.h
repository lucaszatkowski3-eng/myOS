#pragma once
#include <stdint.h>
enum { SYS_EXIT=1, SYS_WRITE=2, SYS_OPEN=3, SYS_READ=4, SYS_CLOSE=5, SYS_SPAWN=6, SYS_SLEEP=7, SYS_NET=8 };
uint64_t syscall_dispatch(uint64_t n,uint64_t a,uint64_t b,uint64_t c,uint64_t d);
