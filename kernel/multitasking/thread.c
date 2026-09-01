#include "thread.h"
#include "../mmu/heap.h"
#include "../stdlib/stdio.h"
#include "../shell/tsh.h"

#define NEW_TASK_KSTACK_SIZE 4096
#define TIME_SLICE_IN_NS 50000000ULL

static volatile uint64_t last_time_check;
static volatile uint64_t IRQ_disable_counter = 0;

static volatile int postpone_task_switches_counter = 0;
static volatile int task_switches_postponed_flag = 0;

static volatile uint64_t CPU_idle_time = 0;

static volatile uint64_t time_slice_remaining = 0;

thread_t* current_task_TCB = NULL;
thread_t* t0 = NULL; //this is our "boot task"
thread_t* shell_task = NULL;
thread_t* terminator_task = NULL; //it will be back

//Ready Task queue
thread_t* first_ready_task = NULL;
thread_t* last_ready_task = NULL;

//Sleeping Task queue
thread_t* first_sleeping_task = NULL;
thread_t* last_sleeping_task = NULL;

//Dead tasks list
thread_t* terminated_task_list = NULL;

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
    terminator_task = create_kernel_task((task_entry_t)reap_all_tasks, "T-800", 2);

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
    uint64_t cur_time = get_uptime_ns();
    uint64_t elapsed = cur_time - last_time_check;
    last_time_check = cur_time;
    if (!current_task_TCB) {
        CPU_idle_time += elapsed;
        return;
    }
    current_task_TCB->time_elapsed += elapsed;
}

//this isn't being used anymore, and will be removed soon
thread_t* get_pid0() {
    return t0;
}

void switch_to_task(thread_t* next_task) {
    //must check this first in case of a postponed switch
    if (postpone_task_switches_counter != 0) {
        task_switches_postponed_flag = 1;
        return;
    }

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

    time_slice_remaining = (first_ready_task != NULL) ? TIME_SLICE_IN_NS : 0;
 
    context_switch(next_task); 
}

void schedule(void) {
    thread_t* task;

    if (postpone_task_switches_counter != 0) {
        task_switches_postponed_flag = 1;
        return;
    }

    if (first_ready_task != NULL) {
        thread_t* task = first_ready_task;
        first_ready_task = task->next;
        //last_ready_task = (first_ready_task == NULL) ? NULL : last_ready_task;
        switch_to_task(task);
    } else if (current_task_TCB->state == THREAD_RUNNING) {
        time_slice_remaining = 0;
    } else {
        task = current_task_TCB;
        current_task_TCB = NULL;
        uint64_t idle_start_time = get_uptime_ns();

        do {
            __asm__ volatile("sti");
            __asm__ volatile("hlt");
            __asm__ volatile("cli");
        } while (first_ready_task == NULL);

        current_task_TCB = task;

        task = first_ready_task;
        first_ready_task = task->next;
        if(task != current_task_TCB) {
            switch_to_task(task);
        }
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

//misc lock with task switch disable
void lock_stuff(void) {
    __asm__ volatile("cli");
    IRQ_disable_counter++;
    postpone_task_switches_counter++;
}

void unlock_stuff(void) {
    postpone_task_switches_counter--;
    if (postpone_task_switches_counter == 0) {
        if (task_switches_postponed_flag != 0) {
            task_switches_postponed_flag = 0;
            schedule();
        }
    }
    IRQ_disable_counter--;
    if (IRQ_disable_counter == 0) __asm__ volatile("sti");
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

//Reminder to future self, do not call switch_to_task directly here, due to irq timer stuff
void unblock_task(thread_t* task) {
    lock_schedule();
    task->state = THREAD_READY;
    task->next = NULL;

    if (last_ready_task == NULL) {
        first_ready_task = task;
        last_ready_task = task;
    } else {
        last_ready_task->next = task;
        last_ready_task = task;
    }

    unlock_schedule();
}

//abs time to avoid drift
void nano_sleep_until(uint64_t wake_time_ns) {
    /*
        Protects the sleeping list from the timer IRQ handler walking/popping
        it concurrently, and (using postponment) makes sure our
        call into block_task() below can't get mixed up with anyone else
        currently in the middle of waking tasks.
    */
    lock_stuff();

    if(wake_time_ns <= get_uptime_ns()) {unlock_stuff(); return;}

    thread_t* task = current_task_TCB;
    task->wake_time = wake_time_ns;
    task->next = NULL;

    if (first_sleeping_task == NULL || wake_time_ns < first_sleeping_task->wake_time) {
        task->next = first_sleeping_task;
        first_sleeping_task = task;
        if (last_sleeping_task == NULL) {
            last_sleeping_task = task;
        }
    } else {
        thread_t* cur = first_sleeping_task;
        while (cur->next != NULL && cur->next->wake_time <= wake_time_ns) {
            cur = cur->next;
        }
        task->next = cur->next;
        cur->next = task;
        if (task->next == NULL) {
            last_sleeping_task = task;
        }
    }

    //block till enough time passes then pops
    block_task(THREAD_SLEEPING);

    unlock_stuff();
}

void nano_sleep(uint64_t nanoseconds) {
    nano_sleep_until(get_uptime_ns() + nanoseconds);
}

void sleep_seconds(uint64_t seconds) {
    nano_sleep(seconds * 1000000000ULL);
}

// Must be called periodically by the timer IRQ handler
void timer_check_sleeping_tasks(void) {
    lock_stuff();

    uint64_t now = get_uptime_ns();

    while (first_sleeping_task != NULL && first_sleeping_task->wake_time <= now) {
        thread_t* task = first_sleeping_task;
        first_sleeping_task = task->next;
        if (first_sleeping_task == NULL) {
            last_sleeping_task = NULL;
        }
        task->next = NULL;

        unblock_task(task);
    }

    unlock_stuff();
}

void scheduler_time_slice_tick(void) {
    lock_stuff();
    if (current_task_TCB != NULL && time_slice_remaining != 0) {
        uint64_t ns_elapsed = get_ns_per_tick();
 
        if (time_slice_remaining <= ns_elapsed) {
            schedule();
        } else {
            time_slice_remaining -= ns_elapsed;
        }
    }
    unlock_stuff();
}

void terminate_task(void) {
    lock_stuff();

    lock_schedule();
    current_task_TCB->next = terminated_task_list;
    terminated_task_list = current_task_TCB;
    unlock_schedule();

    block_task(THREAD_DEAD);

    unblock_task(terminator_task);

    unlock_stuff();
}

void reap_all_tasks(void) {
    thread_t* task;
    lock_stuff();
    while(terminated_task_list != NULL) {
        task = terminated_task_list;
        terminated_task_list = task->next;
        reap_task(task);
    }
    block_task(THREAD_PAUSED);
    unlock_stuff();
}

void reap_task(thread_t* task) {
    kfree(task->rsp0 - NEW_TASK_KSTACK_SIZE);
    kfree(task);
}

void task1() {
    while (1) {
        //printf("%d\n", time_slice_remaining);
        
        //debug_print("EDENNNNNN\n");
        //yield();
    }
}

void task2() {
    while (1) {
        printf("Tomerrrrr\n");
        yield();
    }
}

SEMAPHORE *create_semaphore(int max) {
    SEMAPHORE * semaphore;

    semaphore = kmalloc(sizeof(SEMAPHORE));
    if(semaphore != NULL) {
        semaphore->max_count = max;
        semaphore->current_count = 0;
        semaphore->first_waiting_task = NULL;
        semaphore->last_waiting_task = NULL;
    }
}

SEMAPHORE *create_mutex(void) {
    return create_semaphore(1);
}

void acquire_semaphore(SEMAPHORE * semaphore) {
    lock_stuff();
    if(semaphore->current_count < semaphore->max_count) {
        // We can acquire now
        semaphore->current_count++;
    } else {
        // We have to wait
        current_task_TCB->next = NULL;
        if(semaphore->first_waiting_task == NULL) {
            semaphore->first_waiting_task = current_task_TCB;
        } else {
            semaphore->last_waiting_task->next = current_task_TCB;
        }
        semaphore->last_waiting_task = current_task_TCB;
        block_task(THREAD_WAITING_FOR_LOCK);    // This task will be unblocked when it can acquire the semaphore
    }
    unlock_stuff();
}

void acquire_mutex(SEMAPHORE * semaphore) {
    acquire_semaphore(semaphore);
}

void release_semaphore(SEMAPHORE * semaphore) {
    lock_stuff();

    if(semaphore->first_waiting_task != NULL) {
        // We need to wake up the first task that was waiting for the semaphore
        // Note: "semaphore->current_count" remains the same (this task leaves and another task enters)

        thread_t *task = semaphore->first_waiting_task;
        semaphore->first_waiting_task = task->next;
        unblock_task(task);
    } else {
        // No tasks are waiting
        semaphore->current_count--;
    }
    unlock_stuff();
}

void release_mutex(SEMAPHORE * semaphore) {
    release_semaphore(semaphore);
}