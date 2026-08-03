#pragma once
#include <stdint.h>
#include <stddef.h>
#include "vmm.h"
typedef struct vmm_context_t vmm_context_t;

typedef void *mspace;

typedef struct heap_t {
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
void heap_free(heap_t *heap, void *ptr);
void* heap_realloc(heap_t *heap, void *ptr, size_t new_bytes);

void init_kheap();
int heap_is_valid(const heap_t *heap);
heap_t* k_heap_status(void);
void* sys_sbrk(ptrdiff_t increment);
void* kmalloc(size_t size);
void  kfree(void *ptr);
void* krealloc(void *ptr, size_t size);

void* sys_mmap_alloc(size_t size);
int sys_munmap_free(void* addr, size_t size);