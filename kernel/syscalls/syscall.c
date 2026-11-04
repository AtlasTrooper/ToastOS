#include "syscall.h"
#include "../multitasking/task.h"
#include "../mmu/vmm.h"
#include "../mmu/heap.h"

#define WRITE_BUFF_SIZE 128
#define ERR ((uint64_t)-1)
#define SYSCALL_COUNT (sizeof(syscall_table) / sizeof(syscall_table[0]))

typedef uint64_t (*syscall_fn_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

struct syscall_entry {
    syscall_fn_t func;
    uint8_t nargs;
};
#pragma region Syscall_Implementations
static uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t len)
{
    if (fd != 1 && fd != 2)
        return ERR;

    if (!vmm_user_range_ok(current_task_TCB->vmm_ctx, buf, len, 0))
        return ERR;

    const char *src = (const char *)buf;
    char *tmp = (char*)kmalloc(WRITE_BUFF_SIZE);
    if (!tmp) return ERR;
    uint64_t done = 0;

    while (done < len) {
        uint64_t n = len - done;
        if (n > WRITE_BUFF_SIZE - 1)
            n = WRITE_BUFF_SIZE - 1;
        for (uint64_t i = 0; i < n; i++)
            tmp[i] = src[done + i];
        tmp[n] = '\0';
        printf("%s", tmp);
        done += n;
    }
    kfree(tmp);
    return done;
}

static uint64_t sys_exit(uint64_t code)
{
    printf("[pid %d exited with code %d]\n", (int)current_task_TCB->pid, (int)code);
    terminate_task();
    for (;;) yield();
    return 0;
}

static uint64_t sys_getpid(void)
{
    return current_task_TCB->pid;
}

static uint64_t sys_yield(void)
{
    yield();
    return 0;
}

static uint64_t sys_sleep(uint64_t milliseconds)
{
    nano_sleep(milliseconds * 1000000ULL);
    return 0;
}
#pragma endregion Syscall_Implementations

static uint64_t wrap_sys_write(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
{
    (void)a4; (void)a5;
    return sys_write(a1, a2, a3);
}

static uint64_t wrap_sys_exit(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
{
    (void)a2; (void)a3; (void)a4; (void)a5;
    return sys_exit(a1);
}

static uint64_t wrap_sys_getpid(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
{
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return sys_getpid();
}

static uint64_t wrap_sys_yield(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
{
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return sys_yield();
}

static uint64_t wrap_sys_sleep(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
{
    (void)a2; (void)a3; (void)a4; (void)a5;
    return sys_sleep(a1);
}

static const struct syscall_entry syscall_table[] = {
    [SYS_WRITE]  = { wrap_sys_write, 3 },
    [SYS_EXIT]   = { wrap_sys_exit,  1 },
    [SYS_GETPID] = { wrap_sys_getpid, 0 },
    [SYS_YIELD]  = { wrap_sys_yield,  0 },
    [SYS_SLEEP]  = { wrap_sys_sleep,  1 },
};

uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5)
{
    if (num >= SYSCALL_COUNT || syscall_table[num].func == NULL) {
        printf("unknown syscall %d\n", (int)num);
        return ERR;
    }

    (void)a4; (void)a5;

    return syscall_table[num].func(a1, a2, a3, a4, a5);
}