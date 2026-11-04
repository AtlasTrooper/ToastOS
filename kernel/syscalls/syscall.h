#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>

#define SYSCALL_TABLE \
    X(0, sys_write,  "SYS_WRITE" , 3, usr_write,    uint64_t, (int fd, const char *buf, size_t len),                  ((uint64_t)fd, (uint64_t)buf, (uint64_t)len, 0, 0)) \
    X(1, sys_exit,   "SYS_EXIT",   1, usr_exit,     void,     (int code),                                             ((uint64_t)code, 0, 0, 0, 0)) \
    X(2, sys_getpid, "SYS_GETPID", 0, usr_getpid,   uint64_t, (void),                                                 (0, 0, 0, 0, 0)) \
    X(3, sys_yield,  "SYS_YIELD",  0, usr_yield,    void,     (void),                                                 (0, 0, 0, 0, 0)) \
    X(4, sys_sleep,  "SYS_SLEEP",  1, usr_sleep_ms, void,     (uint64_t ms),                                          (ms, 0, 0, 0, 0))

void init_syscall(void);
uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5);

static inline uint64_t syscall(uint64_t num, uint64_t a1, uint64_t a2, 
                               uint64_t a3, uint64_t a4, uint64_t a5)
{
    uint64_t ret;
    __asm__ volatile(
        "movq %1, %%rax\n\t"
        "movq %2, %%rdi\n\t"
        "movq %3, %%rsi\n\t"
        "movq %4, %%rdx\n\t"
        "movq %5, %%r10\n\t"
        "movq %6, %%r9\n\t"
        "syscall"
        : "=a"(ret)
        : "g"(num), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5)
        : "rdi", "rsi", "rdx", "r10", "r9", "rcx", "r11", "memory"
    );
    return ret;
}

#define UNPACK_ARGS(a1, a2, a3, a4, a5) a1, a2, a3, a4, a5

#define X(id, func, str, nargs, usr_func, ret_type, params, args) \
static inline ret_type usr_func params { \
    uint64_t _r = syscall(id, UNPACK_ARGS args); \
    if (id == 1) { for(;;); } \
    if (!__builtin_types_compatible_p(ret_type, void)) { \
        return (ret_type)_r; \
    } \
}
SYSCALL_TABLE
#undef X
#undef UNPACK_ARGS

#endif
