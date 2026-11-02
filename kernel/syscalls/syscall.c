#include "syscall.h"
#include "../multitasking/task.h"
#include "../mmu/vmm.h"

#define ERR ((uint64_t)-1)

/* ex. printf can't be called directly from ring3 (varargs, kernel pointers, ring0
 * code), so ring3 gets a write(fd, buf, len) and the kernel prints for it. */
static uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t len)
{
    if (fd != 1 && fd != 2)
        return ERR;

    /* every page of [buf, buf+len) must really be mapped + user-accessible in
     * THIS task's address space, otherwise a bad pointer faults inside the kernel */
    if (!vmm_user_range_ok(current_task_TCB->vmm_ctx, buf, len, 0))
        return ERR;

    const char *src = (const char *)buf;   /* valid: we're running under the caller's CR3 */
    char tmp[128];
    uint64_t done = 0;

    while (done < len) {
        uint64_t n = len - done;
        if (n > sizeof(tmp) - 1)
            n = sizeof(tmp) - 1;
        for (uint64_t i = 0; i < n; i++)
            tmp[i] = src[done + i];
        tmp[n] = '\0';
        printf("%s", tmp);
        done += n;
    }
    return done;
}

static void sys_exit(uint64_t code)
{
    printf("[pid %d exited with code %d]\n", (int)current_task_TCB->pid, (int)code);
    terminate_task();           /* hands us to the reaper, never scheduled again */
    for (;;) yield();
}

uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5)
{
    (void)a4; (void)a5;

    switch (num) {
    case SYS_WRITE:  return sys_write(a1, a2, a3);
    case SYS_EXIT:   sys_exit(a1); return 0;
    case SYS_GETPID: return current_task_TCB->pid;
    case SYS_YIELD:  yield(); return 0;
    case SYS_SLEEP:  nano_sleep(a1 * 1000000ULL); return 0;
    default:
        printf("unknown syscall %d\n", (int)num);
        return ERR;
    }
}
