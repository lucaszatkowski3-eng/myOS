#pragma once
#include <stdint.h>
typedef uint64_t pid_t;
typedef struct { pid_t pid; uint64_t entry; uint64_t stack; uint32_t state; char name[64]; } process_t;
int process_init(void); pid_t process_spawn(const char *path); void process_yield(void);
