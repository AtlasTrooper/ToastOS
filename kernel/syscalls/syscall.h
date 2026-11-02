#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "../stdlib/stdio.h"

/* Syscall numbers */
enum {
    SYS_WRITE  = 0,   /* (fd, buf, len) -> bytes written or -1 */
    SYS_EXIT   = 1,   /* (code) */
    SYS_GETPID = 2,   /* () -> pid */
    SYS_YIELD  = 3,   /* () */
    SYS_SLEEP  = 4,   /* (milliseconds) */
};

/* Kernel side */
void init_syscall(void);   /* asm: sets up EFER/STAR/LSTAR/SFMASK. Call once, after the GDT is loaded. */
uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5);

/* User side: raw syscall wrappers (usable from your ring3 code).
 * The kernel preserves every register except rax/rcx/r11, matching this clobber list. */
static inline uint64_t syscall3(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3)
{
    uint64_t ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(num), "D"(a1), "S"(a2), "d"(a3)
                     : "rcx", "r11", "memory");
    return ret;
}

static inline uint64_t usr_write(int fd, const char *buf, size_t len)
{
    return syscall3(SYS_WRITE, (uint64_t)fd, (uint64_t)buf, (uint64_t)len);
}

static inline void usr_exit(int code)
{
    syscall3(SYS_EXIT, (uint64_t)code, 0, 0);
    for (;;) { }
}

static inline uint64_t usr_getpid(void) { return syscall3(SYS_GETPID, 0, 0, 0); }
static inline void     usr_yield(void)  { syscall3(SYS_YIELD, 0, 0, 0); }
static inline void     usr_sleep_ms(uint64_t ms) { syscall3(SYS_SLEEP, ms, 0, 0); }

#endif
