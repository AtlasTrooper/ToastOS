#pragma once
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"
#include "vmm.h"
#include "memory.h"

typedef void *mspace;

typedef struct heap {
    mspace ms;
    uint64_t base_addr;
    uint64_t end_addr;
    uint64_t limit;
    uint64_t flags;
    vmm_context_t *vmm_ctx;
} heap_t;

heap_t* heap_create(
            vmm_context_t *vmm_ctx,
            uint64_t base,
            uint64_t initial_size,
            uint64_t max_size,
            uint64_t flags);
void* heap_alloc(heap_t *heap, size_t bytes);
void heap_free(heap *heap, void *ptr);
void* heap_realloc(heap_t *heap, void *ptr, size_t new_bytes);

