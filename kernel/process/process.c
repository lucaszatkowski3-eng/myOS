#include "process.h"

static struct process table[PROCESS_MAX];
static uint32_t next_pid = 1;
static uint32_t current = 0;
static int in_callback;

static int streq(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] == 0 && b[i] == 0;
}

void process_init(void) {
    for (uint32_t i = 0; i < PROCESS_MAX; ++i) {
        table[i].pid = 0;
        table[i].state = PROCESS_UNUSED;
        table[i].name[0] = 0;
        table[i].entry = 0;
        table[i].arg = 0;
        table[i].ticks = 0;
        table[i].exit_code = 0;
    }
    next_pid = 1;
    current = 0;
    in_callback = 0;
}

int process_create(const char *name, process_entry entry, void *arg) {
    if (!name || !*name || !entry) return -1;
    for (uint32_t i = 0; i < PROCESS_MAX; ++i) {
        if (table[i].state == PROCESS_UNUSED || table[i].state == PROCESS_EXITED) {
            table[i].pid = next_pid++;
            if (!table[i].pid) table[i].pid = next_pid++;
            table[i].state = PROCESS_READY;
            table[i].entry = entry;
            table[i].arg = arg;
            table[i].ticks = 0;
            table[i].exit_code = 0;
            size_t j = 0;
            for (; j < PROCESS_NAME_MAX - 1 && name[j]; ++j)
                table[i].name[j] = name[j];
            table[i].name[j] = 0;
            return (int)table[i].pid;
        }
    }
    return -1;
}

void process_exit(int code) {
    if (current >= PROCESS_MAX) return;
    table[current].exit_code = code;
    table[current].state = PROCESS_EXITED;
}

void scheduler_tick(void) {
    if (in_callback) return;
    for (uint32_t n = 0; n < PROCESS_MAX; ++n) {
        uint32_t i = (current + 1 + n) % PROCESS_MAX;
        if (table[i].state != PROCESS_READY) continue;
        current = i;
        table[i].state = PROCESS_RUNNING;
        table[i].ticks++;
        in_callback = 1;
        table[i].entry(table[i].arg);
        in_callback = 0;
        if (table[i].state == PROCESS_RUNNING)
            table[i].state = PROCESS_READY;
        return;
    }
}

uint32_t process_current_pid(void) {
    return current < PROCESS_MAX ? table[current].pid : 0;
}

uint32_t process_count(void) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < PROCESS_MAX; ++i)
        if (table[i].state != PROCESS_UNUSED && table[i].state != PROCESS_EXITED) n++;
    return n;
}

const struct process *process_get(uint32_t index) {
    return index < PROCESS_MAX ? &table[index] : 0;
}
