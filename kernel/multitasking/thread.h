#pragma once
#include "../util.h"
#include "../idt/idt.h"
#include "process.h"

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKD,
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
} thread_t;

void init_multitasking();
extern void switch_to_task(thread_t* next_thread);
extern thread_t* current_task_TCB;

typedef void (*task_entry_t) (void);
void kernel_task_startup(void);
thread_t* create_kernel_task(task_entry_t eip, char*name, uint64_t pid);

void task1();
void task2();