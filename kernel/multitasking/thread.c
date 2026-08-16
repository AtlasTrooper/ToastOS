#include "thread.h"
#include "../mmu/heap.h"
#include "../stdlib/stdio.h"
#include "../shell/tsh.h"

#define NEW_TASK_KSTACK_SIZE 4096
static volatile uint64_t last_time_check;
static volatile uint64_t IRQ_disable_counter = 0;

thread_t* current_task_TCB = NULL;
thread_t* t0 = NULL; //this is our "boot task"
thread_t* shell_task = NULL;

//Ready Task queue
thread_t* first_ready_task = NULL;
thread_t* last_ready_task = NULL;

//Test tasks
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

    uint64_t boot_rsp;
    __asm__ volatile ("movq %%rsp, %0" : "=r"(boot_rsp));
    t0->rsp0 = (void*)boot_rsp;

    uint64_t boot_cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r"(boot_cr3));
    t0->cr3 = (void*)boot_cr3;

    current_task_TCB = t0;
    last_time_check = 0;
    update_task_time();

    shell_task = create_kernel_task((task_entry_t)init_shell, "tsh", 1);

    //TEST
    t1 = create_kernel_task((task_entry_t)task1, "tomer", 2);
    t2 = create_kernel_task((task_entry_t)task2, "eden", 3);

    debug_print("[let the multitasking begin!]\n");

    while(1) yield();

}

void kernel_task_startup(void) {
    unlock_schedule();
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
    if(!new_task_stack) return NULL;

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
    new_task->next = NULL;
    new_task->parent = NULL;

    if (last_ready_task == NULL) {
        first_ready_task = new_task;
        last_ready_task = new_task;
    } else {
        last_ready_task->next = new_task;
        last_ready_task = new_task;
    }

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

void switch_to_task(thread_t* next_task) {
    thread_t* prev_task = current_task_TCB;
 
    if (prev_task->state == THREAD_RUNNING) {
        prev_task->state = THREAD_READY;
        prev_task->next = NULL;
 
        if (last_ready_task == NULL) {
            first_ready_task = prev_task;
            last_ready_task = prev_task;
        } else {
            last_ready_task->next = prev_task;
            last_ready_task = prev_task;
        }
    }
 
    next_task->state = THREAD_RUNNING;
 
    context_switch(next_task); 
}

void schedule(void) {
    if (first_ready_task != NULL) {
        thread_t* task = first_ready_task;
        first_ready_task = task->next;
 
        if (first_ready_task == NULL) {
            last_ready_task = NULL;
        }
 
        switch_to_task(task);
    }
}

void lock_schedule(void) {
    __asm__ volatile("cli");
    IRQ_disable_counter++;
}

void unlock_schedule(void) {
    IRQ_disable_counter--;
    if(IRQ_disable_counter == 0) __asm__ volatile("sti");
}

void yield(void) {
    lock_schedule();
    schedule();
    unlock_schedule();
}

void block_task(int reason) {
    lock_schedule();
    current_task_TCB->state = reason;
    schedule();
    unlock_schedule();
}

void unblock_task(thread_t* task) {
    
}


void task1() {
    while (1) {
        printf("Edennnnn\n");
        yield();
    }
}

void task2() {
    while (1) {
        printf("Tomerrrrr\n");
        yield();
    }
}