#include "thread.h"
#include "../mmu/heap.h"
#include "../stdlib/stdio.h"
#include "../shell/tsh.h"

#define NEW_TASK_KSTACK_SIZE 4096
static volatile uint64_t last_time_check;

thread_t* current_task_TCB = NULL;
thread_t* t0 = NULL; //this is our "boot task"
thread_t* shell_task = NULL;

thread_t* first_task = NULL;
thread_t* last_task = NULL; //pointer to end of tasklist

//Test
thread_t* t1 = NULL;
thread_t* t2 = NULL;

void init_multitasking() {

    t0 = (thread_t*)kmalloc(sizeof(thread_t));
    t0->name = "boot";
    t0->pid = 0;
    t0->state = THREAD_RUNNING;
    t0->parent = NULL;
    t0->next = NULL;
    t0->time_elapsed = 0;

    current_task_TCB = t0;
    last_task = t0;
    last_time_check = 0;
    update_task_time();

    shell_task = create_kernel_task((task_entry_t)init_shell, "tsh", 1);

    //TEST
    t1 = create_kernel_task((task_entry_t)task1, "tomer", 2);
    t2 = create_kernel_task((task_entry_t)task2, "eden", 3);

    debug_print("[let the multitasking begin!]\n");

    schedule();
    while(1);

}

void kernel_task_startup(void) {
    __asm__ volatile("sti");

    /*
        might add more stuff here
        potential idea would be to take a func ptr
        param so that tasks can have their own custom
        startup
    */
}

thread_t* create_kernel_task(task_entry_t eip, char*name, uint64_t pid) {
    thread_t* new_task = (thread_t*)kmalloc(sizeof(thread_t));
    if (!new_task) return NULL;

    void* new_task_stack = kmalloc(NEW_TASK_KSTACK_SIZE);
    if(!new_task) return NULL;

    uint64_t stack_top = (uint64_t)new_task_stack + NEW_TASK_KSTACK_SIZE;

    stack_top &= ~0xF;

    uint64_t*stack_ptr = (uint64_t*)stack_top;

    *(--stack_ptr) = (uint64_t)eip;
    *(--stack_ptr) = (uint64_t)kernel_task_startup;

    for(int i = 0; i < 6;i++){*(--stack_ptr) = 0;}
    new_task->name = name;
    new_task->pid = pid;
    new_task->rsp = (void*)stack_ptr;
    new_task->rsp0 = (void*)stack_top;
    new_task->time_elapsed = 0;

    uint64_t cur_cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r"(cur_cr3));
    new_task->cr3 = (void*)cur_cr3;

    new_task->state = THREAD_READY;
    
    last_task->next = new_task;
    last_task = new_task;
    new_task->next = shell_task;

    new_task->parent = NULL;

    return new_task;
}

void update_task_time(void) {
    uint64_t cur_time = get_uptime_ms() * (1000 * 1000); //haha multiplying by a million is crazyyy
    uint64_t elapsed = last_time_check - cur_time;
    last_time_check = cur_time;
    current_task_TCB->time_elapsed += elapsed;
}

thread_t* get_pid0() {
    return t0;
}

void schedule(void) {
    __asm__ volatile("cli");
    switch_to_task(current_task_TCB->next);
    __asm__ volatile("sti");
}

void task1() {
    while (1) {
        printf("Edennnnn\n");
        schedule();
    }
}

void task2() {
    while (1) {
        printf("Tomerrrrr\n");
        schedule();
    }
}