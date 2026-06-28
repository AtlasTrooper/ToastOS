#pragma once
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"
#include "vmm.h"


typedef struct {
    uintptr_t base;
    uintptr_t curr;     /* current break (== base + committed bytes)       */
    uintptr_t mapped;   /* how far we have actually mapped (page-aligned)  */
    int       is_init;
} heap_t;

/*
 * heap_init  – set up a heap starting at `base`.
 *   Call once per address space.  For the kernel heap use init_kheap()
 *   which picks the right base automatically.
 */
void  heap_init(heap_t *heap, uintptr_t base);

/*
 * heap_sbrk  – sbrk(2)-style break adjustment.
 *   inc > 0 : grow by inc bytes, map new pages as needed, return OLD break
 *   inc < 0 : shrink by |inc| bytes, unmap freed pages, return NEW break
 *   inc = 0 : return current break (no change)
 *   Returns (void*)-1 on error (OOM or underflow).
 */
void *heap_sbrk(heap_t *heap, int64_t inc);

/*
 * init_kheap – initialise the global kernel heap.
 *   Base is derived from the PMM (right after the bitmap).
 */
void  init_kheap(void);

void *kmalloc (size_t size);
void  kfree   (void *ptr);
void *krealloc(void *ptr, size_t size);
void *kcalloc (size_t nmemb, size_t size);

/* Returns a pointer to the raw kernel heap_t (for debugging). */
heap_t *k_heap_status(void);
