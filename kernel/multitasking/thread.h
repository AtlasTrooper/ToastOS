#pragma once
#include "../util.h"
#include "../idt/idt.h"
#include "process.h"
#include "../drivers/timer.h"
typedef enum {
    THREAD_READY, //ready to run
    THREAD_RUNNING, //actively running
    THREAD_BLOCKED,
    THREAD_WAITING_FOR_LOCK,
    THREAD_PAUSED,
    THREAD_SLEEPING, //sleeping a set amount of time
    THREAD_DEAD //awaiting a reap
} thread_state_t;

typedef struct PACKED thread_t {
    char *name;
    uint64_t pid;
    void* rsp;
    void* rsp0;
    void* cr3;
    thread_state_t state;    
    struct thread_t *next;   
    struct thread_t *parent; 
    uint64_t time_elapsed;
    uint64_t wake_time; //ns since boot (8 bytes)
} thread_t;

typedef struct SEMAPHORE{
    int max_count;
    int current_count;
    thread_t* first_waiting_task;
    thread_t* last_waiting_task;

}SEMAPHORE;

void init_multitasking();
void switch_to_task(thread_t* next_task);
extern void context_switch(thread_t* next_thread);

extern thread_t* current_task_TCB;
extern thread_t* first_ready_task;
extern thread_t* last_ready_task;
extern thread_t* first_sleeping_task;
extern thread_t* last_sleeping_task;

typedef void (*task_entry_t) (void);
void kernel_task_startup(void);
thread_t* create_kernel_task(task_entry_t eip, char*name, uint64_t pid);
void update_task_time(void);
void schedule(void);

//Test functions
void task1();
void task2();

//Terminal time poll
thread_t* get_pid0();

//Schedule lock functions
void lock_schedule(void);
void unlock_schedule(void);

void lock_stuff(void);
void unlock_stuff(void);

void block_task(int reason);
void unblock_task(thread_t* task);

void yield(void);

void nano_sleep_until(uint64_t wake_time_ns);
void nano_sleep(uint64_t nanoseconds);
void sleep_seconds(uint64_t seconds);

//PIT scheduler ticks
void timer_check_sleeping_tasks(void);
void scheduler_time_slice_tick(void);

//Moves task to dead task list
void terminate_task(void);//say hello to my little friend
//frees up the dead task list
void reap_all_tasks(void);
//frees individual task and it's memory
void reap_task(thread_t* task);//the grim sweeper

SEMAPHORE* create_semaphore(int max);
SEMAPHORE* create_mutex(void);
void acquire_semaphore(SEMAPHORE* semaphore);
void acquire_mutex(SEMAPHORE* semaphore);
void release_semaphore(SEMAPHORE* semaphore);
void release_mutex(SEMAPHORE* semaphore);