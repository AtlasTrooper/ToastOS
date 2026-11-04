#include "syscall.h"
#include "../multitasking/task.h"
#include "../mmu/vmm.h"
#include "../mmu/heap.h"

#define WRITE_BUFF_SIZE 128
#define ERR ((uint64_t)-1)

typedef uint64_t (*syscall_fn_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

#define X(id, func, str, nargs, usr_func, ret_type, params, args) uint64_t func(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
SYSCALL_TABLE
#undef X

typedef enum {
    #define X(id, func, str, nargs, usr_func, ret_type, params, args) SYSCALL_##func = id,
    SYSCALL_TABLE
    #undef X
    SYSCALL_COUNT
} syscall_id_t;

typedef struct {
    syscall_fn_t func;
    const char *name;
    int num_args;
} syscall_entry_t;

const syscall_entry_t syscall_table[] = {
    #define X(id, func, str, nargs, usr_func, ret_type, params, args) [id] = { func, str, nargs },
    SYSCALL_TABLE
    #undef X
};

#pragma region Syscall_Implementations
uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t len, uint64_t nop4, uint64_t nop5)
{
    (void)nop4; (void)nop5;
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

uint64_t sys_exit(uint64_t code, uint64_t nop2, uint64_t nop3, uint64_t nop4, uint64_t nop5)
{   
    (void)nop2; (void)nop3; (void)nop4; (void)nop5;

    printf("[pid %d exited with code %d]\n", (int)current_task_TCB->pid, (int)code);
    terminate_task();
    for (;;) yield();
    return 0;
}

uint64_t sys_getpid(uint64_t nop1, uint64_t nop2, uint64_t nop3, uint64_t nop4, uint64_t nop5)
{
    (void)nop1; (void)nop2; (void)nop3; (void)nop4; (void)nop5;
    return current_task_TCB->pid;
}

uint64_t sys_yield(uint64_t nop1, uint64_t nop2, uint64_t nop3, uint64_t nop4, uint64_t nop5)
{
    (void)nop1; (void)nop2; (void)nop3; (void)nop4; (void)nop5;
    yield();
    return 0;
}

uint64_t sys_sleep(uint64_t milliseconds, uint64_t nop2, uint64_t nop3, uint64_t nop4, uint64_t nop5)
{
    (void)nop2; (void)nop3; (void)nop4; (void)nop5;
    nano_sleep(milliseconds * 1000000ULL);
    return 0;
}
#pragma endregion Syscall_Implementations

uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5)
{
    if (num >= SYSCALL_COUNT || syscall_table[num].func == NULL) {
        printf("unknown syscall %d\n", (int)num);
        return ERR;
    }

    return syscall_table[num].func(a1, a2, a3, a4, a5);
}
