#pragma once
#include <stdint.h>

enum syscall_number {
    SYS_YIELD = 0,
    SYS_EXIT = 1,
    SYS_GETPID = 2,
    SYS_WRITE = 3
};

struct syscall_result {
    int64_t value;
    int64_t error;
};

struct syscall_result syscall_dispatch(uint64_t number,
                                        uint64_t a1,
                                        uint64_t a2,
                                        uint64_t a3);
