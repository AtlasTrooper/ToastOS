#pragma once
#include "../util.h"
#include "../idt/idt.h"
#include "../drivers/timer.h"

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,          //generic blocked state, idk what to do with this yet maybe I'll remove
    TASK_WAITING_FOR_LOCK, //blocked in a semaphore/mutex wait queue
    TASK_PAUSED,           //sleeping for a non set amt of time, not on any queue (ex.. the reaper between runs)
    TASK_SLEEPING,         //sleeping a set amount of time
    TASK_DEAD              //awaiting a reap
} task_state_t;

typedef struct SEMAPHORE SEMAPHORE;

typedef struct PACKED task_t {
    char *name;
    uint64_t pid;
    void* rsp;
    void* rsp0;
    void* cr3;
    task_state_t state;
    struct task_t *next;
    struct task_t *parent;
    uint64_t time_elapsed;
    uint64_t wake_time;      //ns since boot

    SEMAPHORE* waiting_for;
    void* kstack_base;
} task_t;

typedef struct SEMAPHORE {
    int max_count;
    int current_count;
    task_t* first_waiting_task;
    task_t* last_waiting_task;
};

//Elder tasks (cannot be killed)
extern task_t* t0;
extern task_t* shell_task;
extern task_t* terminator_task;

//Task apparatus maintainers
extern task_t* current_task_TCB;
extern task_t* first_ready_task;
extern task_t* last_ready_task;
extern task_t* first_sleeping_task;
extern task_t* last_sleeping_task;
extern task_t* terminated_task_list;

typedef void (*task_entry_t) (void);

#pragma region Scheduler main functions
void init_multitasking(void);
void switch_to_task(task_t* next_task);
extern void context_switch(task_t* next_task);

void kernel_task_startup(void);
task_t* create_kernel_task(task_entry_t eip, char* name);
void update_task_time(void);
void schedule(void);
void yield(void);
#pragma endregion

#pragma region Schedule lock functions
void lock_schedule(void);    //cli, IRQ-disable-count only
void unlock_schedule(void);

void lock_stuff(void);       //cli + IRQ-disable-count + postpones task switches
void unlock_stuff(void);

void block_task(int reason);
void unblock_task(task_t* task);
#pragma endregion

#pragma region sleep
void nano_sleep_until(uint64_t wake_time_ns);
void nano_sleep(uint64_t nanoseconds);
void sleep_seconds(uint64_t seconds);
#pragma endregion

#pragma region PIT scheduler ticks
void timer_check_sleeping_tasks(void);
void scheduler_time_slice_tick(void);
#pragma endregion

#pragma region Task_Killing
//Moves the calling task onto the dead list and wakes the reaper.
void terminate_task(void);            //say hello to my little friend
//Kills an arbitrary task by pid, from any state (ready/sleeping/waiting/self).
//Returns 0 on success, -1 if the pid doesn't exist or isn't killable.
int kill_task_by_pid(uint64_t pid);
//Frees everything on the dead list, then parks until woken again.
void reap_all_tasks(void);
//Frees an individual task and its kernel stack.
void reap_task(task_t* task);         //the grim sweeper
#pragma endregion

#pragma region Semaphores and mutexes
SEMAPHORE* create_semaphore(int max);
SEMAPHORE* create_mutex(void);
void acquire_semaphore(SEMAPHORE* semaphore);
void acquire_mutex(SEMAPHORE* semaphore);
void release_semaphore(SEMAPHORE* semaphore);
void release_mutex(SEMAPHORE* semaphore);
#pragma endregion