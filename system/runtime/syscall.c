#include "syscall.h"
#include "../../kernel/process/process.h"

static struct syscall_result result(int64_t value, int64_t error) {
    struct syscall_result r = { value, error };
    return r;
}

struct syscall_result syscall_dispatch(uint64_t number,
                                        uint64_t a1,
                                        uint64_t a2,
                                        uint64_t a3) {
    (void)a2;
    (void)a3;
    switch (number) {
        case SYS_YIELD:
            return result(0, 0);
        case SYS_EXIT:
            process_exit((int)a1);
            return result(0, 0);
        case SYS_GETPID:
            return result((int64_t)process_current_pid(), 0);
        case SYS_WRITE:
            /* User-memory validation and terminal/file descriptors are deliberately
               handled by the next user-mode layer. Never dereference a user pointer here. */
            return result(-1, 38);
        default:
            return result(-1, 38);
    }
}
