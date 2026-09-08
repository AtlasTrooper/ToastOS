#include "task.h"
#include "pidTable.h"
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

task_t* current_task_TCB = NULL;
task_t* t0 = NULL; //this is our "boot task"
task_t* shell_task = NULL;
task_t* terminator_task = NULL; //it will be back

//Ready Task queue
task_t* first_ready_task = NULL;
task_t* last_ready_task = NULL;

//Sleeping Task queue
task_t* first_sleeping_task = NULL;
task_t* last_sleeping_task = NULL;

//Dead tasks list
task_t* terminated_task_list = NULL;

#pragma region ds helpers 
static void ready_push(task_t* task) {
    task->next = NULL;
    if (last_ready_task) {
        last_ready_task->next = task;
    } else {
        first_ready_task = task;
    }
    last_ready_task = task;
}

static task_t* ready_pop(void) {
    if (!first_ready_task) return NULL;
    task_t* task = first_ready_task;
    first_ready_task = task->next;
    if (!first_ready_task) last_ready_task = NULL; //don't leave this dangling
    task->next = NULL;
    return task;
}

/*
Arbitrary removal - used by kill_task_by_pid() to yank a non-current
task out of the ready queue.
*/
static void ready_remove(task_t* task) {
    if (task == first_ready_task) { ready_pop(); return; }
    task_t* tracker = first_ready_task;
    while (tracker && tracker->next != task) tracker = tracker->next;
    if (tracker) {
        tracker->next = task->next;
        if (task == last_ready_task) last_ready_task = tracker;
        task->next = NULL;
    }
}

static void sleep_push(task_t* task) {
    task->next = NULL;

    if (!first_sleeping_task || task->wake_time < first_sleeping_task->wake_time) {
        task->next = first_sleeping_task;
        first_sleeping_task = task;
        if (!last_sleeping_task) last_sleeping_task = task;
        return;
    }

    task_t* tracker = first_sleeping_task;
    //NOTE: must check tracker->next before dereferencing it - walking
    //off the end of the list here was reading through a NULL pointer.
    while (tracker->next && tracker->next->wake_time <= task->wake_time) {
        tracker = tracker->next;
    }
    task->next = tracker->next;
    tracker->next = task;
    if (!task->next) last_sleeping_task = task;
}

static task_t* sleep_pop(void) {
    if (!first_sleeping_task) return NULL;
    task_t* task = first_sleeping_task;
    first_sleeping_task = task->next;
    if (!first_sleeping_task) last_sleeping_task = NULL;
    task->next = NULL;
    return task;
}

static void sleep_remove(task_t* task) {
    if (task == first_sleeping_task) { sleep_pop(); return; }
    task_t* tracker = first_sleeping_task;
    while (tracker && tracker->next != task) tracker = tracker->next;
    if (tracker) {
        tracker->next = task->next;
        if (task == last_sleeping_task) last_sleeping_task = tracker;
        task->next = NULL;
    }
}

static void sem_push(SEMAPHORE* sem, task_t* task) {
    task->next = NULL;
    task->waiting_for = sem;
    if (sem->last_waiting_task) {
        sem->last_waiting_task->next = task;
    } else {
        sem->first_waiting_task = task;
    }
    sem->last_waiting_task = task;
}

static task_t* sem_pop(SEMAPHORE* sem) {
    task_t* task = sem->first_waiting_task;
    if (task) {
        sem->first_waiting_task = task->next;
        if (!sem->first_waiting_task) sem->last_waiting_task = NULL;
        task->next = NULL;
        task->waiting_for = NULL;
    }
    return task;
}

static void sem_remove(SEMAPHORE* sem, task_t* task) {
    if (task == sem->first_waiting_task) { sem_pop(sem); return; }
    task_t* tracker = sem->first_waiting_task;
    while (tracker && tracker->next != task) tracker = tracker->next;
    if (tracker) {
        tracker->next = task->next;
        if (task == sem->last_waiting_task) sem->last_waiting_task = tracker;
        task->next = NULL;
        task->waiting_for = NULL;
    }
}
#pragma endregion

void init_multitasking(void) {

    pid_table_init();

    t0 = (task_t*)kmalloc(sizeof(task_t));
    t0->name = "boot";
    t0->pid = 0;
    t0->state = TASK_RUNNING;
    t0->parent = NULL;
    t0->next = NULL;
    t0->time_elapsed = 0;
    t0->waiting_for = NULL;
    t0->kstack_base = NULL; //t0 runs on the original boot stack, nothing to free later

    uint64_t boot_rsp;
    __asm__ volatile ("movq %%rsp, %0" : "=r"(boot_rsp));
    t0->rsp0 = (void*)boot_rsp;

    uint64_t boot_cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r"(boot_cr3));
    t0->cr3 = (void*)boot_cr3;

    current_task_TCB = t0;
    last_time_check = 0;
    update_task_time();

    pid_table_insert(t0);

    shell_task = create_kernel_task((task_entry_t)init_shell, "tsh");
    terminator_task = create_kernel_task((task_entry_t)reap_all_tasks, "T-800");

    debug_print("[let the multitasking begin!]\n");

    while (1) yield();
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

task_t* create_kernel_task(task_entry_t eip, char* name) {
    task_t* new_task = (task_t*)kmalloc(sizeof(task_t));
    if (!new_task) return NULL;

    void* new_task_stack = kmalloc(NEW_TASK_KSTACK_SIZE);
    if (!new_task_stack) {
        kfree(new_task); //used to leak the TCB here on allocation failure (:
        return NULL;
    }

    uint64_t stack_top = (uint64_t)new_task_stack + NEW_TASK_KSTACK_SIZE;
    stack_top &= ~0xF;

    uint64_t* stack_ptr = (uint64_t*)stack_top;

    *(--stack_ptr) = (uint64_t)eip;
    *(--stack_ptr) = (uint64_t)kernel_task_startup;

    for (int i = 0; i < 6; i++) { *(--stack_ptr) = 0; }

    new_task->name = name;
    new_task->rsp = (void*)stack_ptr;
    new_task->rsp0 = (void*)stack_top;
    new_task->kstack_base = new_task_stack; //remember the *real* base for freeing later
    new_task->time_elapsed = 0;
    new_task->waiting_for = NULL;

    uint64_t cur_cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r"(cur_cr3));
    new_task->cr3 = (void*)cur_cr3;

    new_task->state = TASK_READY;
    new_task->next = NULL;
    new_task->parent = NULL;

    new_task->pid = pid_alloc();

    lock_schedule();
    ready_push(new_task);
    unlock_schedule();

    pid_table_insert(new_task);

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

void switch_to_task(task_t* next_task) {
    //must check this first in case of a postponed switch
    if (postpone_task_switches_counter != 0) {
        task_switches_postponed_flag = 1;
        return;
    }

    task_t* prev_task = current_task_TCB;

    if (prev_task->state == TASK_RUNNING) {
        prev_task->state = TASK_READY;
        ready_push(prev_task);
    }

    next_task->state = TASK_RUNNING;

    /*
    prevents a lone task from reaching 0 time left and hogging till yield
    due to the way scheduler preempts via time slice
    */
    time_slice_remaining = TIME_SLICE_IN_NS;

    context_switch(next_task);
}

void schedule(void) {
    if (postpone_task_switches_counter != 0) {
        task_switches_postponed_flag = 1;
        return;
    }

    if (first_ready_task != NULL) {
        task_t* task = ready_pop();
        switch_to_task(task);
        return;
    }

    if (current_task_TCB->state == TASK_RUNNING) {
        //nobody else is ready - keep running, nothing to do
        return;
    }

    //current task blocked/slept/died and nothing is ready so go idle
    task_t* parked = current_task_TCB;
    current_task_TCB = NULL;

    do {
        __asm__ volatile("sti");
        __asm__ volatile("hlt");
        __asm__ volatile("cli");
    } while (first_ready_task == NULL);

    current_task_TCB = parked;

    task_t* task = ready_pop();
    if (task != current_task_TCB) {
        switch_to_task(task);
    }
}

void lock_schedule(void) {
    __asm__ volatile("cli");
    IRQ_disable_counter++;
}

void unlock_schedule(void) {
    IRQ_disable_counter--;
    if (IRQ_disable_counter == 0) __asm__ volatile("sti");
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
void unblock_task(task_t* task) {
    lock_schedule();
    task->state = TASK_READY;
    ready_push(task);
    unlock_schedule();
}

//abs time to avoid drift
void nano_sleep_until(uint64_t wake_time_ns) {
    /*
        Protects the sleeping list from the timer IRQ handler walking/popping
        it concurrently, and (using postponement) makes sure our
        call into block_task() below can't get mixed up with anyone else
        currently in the middle of waking tasks.
    */
    lock_stuff();

    if (wake_time_ns <= get_uptime_ns()) { unlock_stuff(); return; }

    current_task_TCB->wake_time = wake_time_ns;
    sleep_push(current_task_TCB);

    //block till enough time passes, then timer_check_sleeping_tasks() pops us
    block_task(TASK_SLEEPING);

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
        task_t* task = sleep_pop();
        unblock_task(task);
    }

    unlock_stuff();
}

void scheduler_time_slice_tick(void) {
    lock_stuff();

    if (current_task_TCB != NULL) {
        uint64_t ns_elapsed = get_ns_per_tick();

        if (time_slice_remaining <= ns_elapsed) {
            if (first_ready_task != NULL) {
                schedule();
            } else { // no task to switch to, reset time slice to prevent a stuck till yield
                time_slice_remaining = TIME_SLICE_IN_NS;
            }
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

    block_task(TASK_DEAD);

    unblock_task(terminator_task);

    unlock_stuff();
}

int kill_task_by_pid(uint64_t pid) {
    task_t* task = pid_table_lookup(pid);
    if (task == NULL) return -1;

    if (task == current_task_TCB) {
        terminate_task();
        return 0;
    }

    //protec de elder tasks
    if (task == t0 || task == terminator_task) return -1;

    lock_stuff();

    if (task->state == TASK_DEAD) {
        unlock_stuff();
        return -1;
    }

    switch (task->state) {
        case TASK_READY:
            ready_remove(task);
            break;
        case TASK_SLEEPING:
            sleep_remove(task);
            break;
        case TASK_WAITING_FOR_LOCK:
            if (task->waiting_for != NULL) {
                sem_remove(task->waiting_for, task);
            }
            break;
        default:
            //TASK_PAUSED / TASK_BLOCKED
            break;
    }

    task->state = TASK_DEAD;
    task->next = terminated_task_list;
    terminated_task_list = task;

    unblock_task(terminator_task);

    unlock_stuff();
    return 0;
}

void reap_all_tasks(void) {
    while (1) {
        lock_stuff();

        while (terminated_task_list != NULL) {
            task_t* task = terminated_task_list;
            terminated_task_list = task->next;
            reap_task(task);
        }

        block_task(TASK_PAUSED);

        unlock_stuff();
    }
}

void reap_task(task_t* task) {
    if(!task)return;

    pid_table_remove(task->pid);
    if (task->kstack_base != NULL) {
        kfree(task->kstack_base);
    }
    kfree(task);
    pid_release(pid);
}

SEMAPHORE* create_semaphore(int max) {
    SEMAPHORE* semaphore = kmalloc(sizeof(SEMAPHORE));
    if (semaphore != NULL) {
        semaphore->max_count = max;
        semaphore->current_count = 0;
        semaphore->first_waiting_task = NULL;
        semaphore->last_waiting_task = NULL;
    }
    return semaphore;
}

SEMAPHORE* create_mutex(void) {
    return create_semaphore(1);
}

void acquire_semaphore(SEMAPHORE* semaphore) {
    lock_stuff();
    if (semaphore->current_count < semaphore->max_count) {
        // can acquire now
        semaphore->current_count++;
    } else {
        // We have to wait
        sem_push(semaphore, current_task_TCB);
        block_task(TASK_WAITING_FOR_LOCK); // unblocked once we can acquire the semaphore
    }
    unlock_stuff();
}

void acquire_mutex(SEMAPHORE* semaphore) {
    acquire_semaphore(semaphore);
}

void release_semaphore(SEMAPHORE* semaphore) {
    lock_stuff();

    if (semaphore->first_waiting_task != NULL) {
        task_t* task = sem_pop(semaphore);
        unblock_task(task);
    } else {
        semaphore->current_count--;
    }

    unlock_stuff();
}

void release_mutex(SEMAPHORE* semaphore) {
    release_semaphore(semaphore);
}