#pragma once
#include "../util.h"
#include "../idt/idt.h"
#include "process.h"
#include "../drivers/timer.h"
typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKD,
    THREAD_PAUSED,
    THREAD_DEAD
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

void timer_check_sleeping_tasks(void);