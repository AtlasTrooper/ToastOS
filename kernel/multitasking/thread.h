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

struct thread;

typedef struct thread_t {
    uint64_t tid;
    process_t *parent;
    
    uint64_t rsp;
    uint64_t kstack_base;     // Base address of the thread's kernel stack
    uint64_t kstack_size;     // Size of the kernel stack (usually 4KiB or 8KiB)
    
    uint64_t time_slice;      // Ticks remaining before this thread is preempted
    thread_state_t state;     // Current execution state
    
    struct thread *next;      // Pointer for the scheduler's Ready Queue
} thread_t;
