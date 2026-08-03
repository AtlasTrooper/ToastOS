#pragma once
#include <stdint.h>
#include <stddef.h>
#include "vmm.h"
typedef struct vmm_context_t vmm_context_t;

typedef struct heap_t {
    uint64_t base_addr;
    uint64_t brk;
    uint64_t mapped_end;
    uint64_t limit;
    uint64_t flags;
    vmm_context_t *vmm_ctx;
} heap_t;

void init_kheap(void);
heap_t* k_heap_status(void);
int heap_is_valid(const heap_t *heap);

/*
sbrk-style connected to dlmalloc via MORECORE.
increment > 0: grow the break, mapping new pages as needed.
increment < 0: shrink the break, unmapping and freeing pages no longer used.
increment == 0: no-op, just returns the current break.
Returns the *previous* break on success, or (void*)-1 on failure.
*/
void* sys_sbrk(intptr_t increment);

void* kmalloc(size_t size);
void  kfree(void *ptr);
void* krealloc(void *ptr, size_t size);

/*
Force dlmalloc to release any free pages sitting at the top of the heap
back to the VMM/PMM immediately, rather than waiting for the trim
threshold to be crossed organically. Returns 1 if memory was released,
0 otherwise.
*/
int kheap_trim(size_t pad);