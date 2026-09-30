#pragma once
#include <stdint.h>
#include <stddef.h>

#define PROCESS_MAX 32
#define PROCESS_NAME_MAX 32

enum process_state {
    PROCESS_UNUSED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_EXITED
};

typedef void (*process_entry)(void *arg);

struct process {
    uint32_t pid;
    enum process_state state;
    char name[PROCESS_NAME_MAX];
    process_entry entry;
    void *arg;
    uint64_t ticks;
    int exit_code;
};

void process_init(void);
int process_create(const char *name, process_entry entry, void *arg);
void process_exit(int code);
void scheduler_tick(void);
uint32_t process_current_pid(void);
uint32_t process_count(void);
const struct process *process_get(uint32_t index);
