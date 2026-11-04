#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "../stdlib/stdio.h"

/* Syscall num/id */
enum {
    SYS_WRITE  = 0,
    SYS_EXIT   = 1,
    SYS_GETPID = 2,
    SYS_YIELD  = 3,
    SYS_SLEEP  = 4,
};

/* Kernel side */
void init_syscall(void);
uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5);

/* User side: clean inline assembly wrappers matching x86_64 calling conventions 
 * Registers: rax=num, rdi=a1, rsi=a2, rdx=a3, r10=a4, r9=a5
 */
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

/* User helpers */
static inline uint64_t usr_write(int fd, const char *buf, size_t len)
{
    return syscall(SYS_WRITE, (uint64_t)fd, (uint64_t)buf, (uint64_t)len, 0, 0);
}

static inline void usr_exit(int code)
{
    syscall(SYS_EXIT, (uint64_t)code, 0, 0, 0, 0);
    for (;;) { }
}

static inline uint64_t usr_getpid(void) 
{ 
    return syscall(SYS_GETPID, 0, 0, 0, 0, 0); 
}

static inline void usr_yield(void)  
{ 
    syscall(SYS_YIELD, 0, 0, 0, 0, 0); 
}

static inline void usr_sleep_ms(uint64_t ms) 
{ 
    syscall(SYS_SLEEP, ms, 0, 0, 0, 0); 
}

#endif