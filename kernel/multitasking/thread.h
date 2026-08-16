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
    char *name;              // Offset  0 (8 bytes)
    uint64_t pid;            // Offset  8 (8 bytes)
    void* rsp;               // Offset 16 (8 bytes) - Kernel stack pointer
    void* rsp0;              // Offset 24 (8 bytes) - Ring 0 stack top for TSS
    void* cr3;               // Offset 32 (8 bytes) - Page directory base
    thread_state_t state;    // Offset 40 (4 bytes)
    struct thread_t *next;   // Offset 44 (8 bytes)
    struct thread_t *parent; // Offset 52 (8 bytes)

    uint64_t time_elapsed; //Offset 60
} thread_t;

void init_multitasking();
void switch_to_task(thread_t* next_task);
extern void context_switch(thread_t* next_thread);

extern thread_t* current_task_TCB;
extern thread_t* first_ready_task;
extern thread_t* last_ready_task;

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

void block_task(int reason);
void unblock_task(thread_t* task);

void yield(void);