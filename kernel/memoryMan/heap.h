#pragma once
#include "memory.h"
#include "pmm.h"
#include "vmm.h"
#include "../qol.h"

typedef struct heap_t{
    uintptr_t s;
    uintptr_t curr;
    uintptr_t max;
    int is_init;
}heap_t;

heap_t* k_heap_status();
void init_Kheap(uintptr_t heap_s);
void *heapafus(int32_t inc, heap_t* heap);
