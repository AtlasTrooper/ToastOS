#include "heap.h"

static heap_t k_heap;

heap_t* k_heap_status() {
    if (k_heap.is_init == 0) {
        return NULL;
    }

    return &k_heap;
}

void init_Kheap(uintptr_t heap_s) {
    k_heap.s = heap_s;
    k_heap.curr = heap_s;
    k_heap.max = CEIL(heap_s, PAGE_SIZE);
    k_heap.is_init = 1;
}

void *heapafus(int32_t inc, heap_t* heap) {
    if(!heap) return (void*)-1;
    uintptr_t old_curr = heap->curr;
    uintptr_t new_curr = heap->curr + inc;

    /*
    Nechalek lemikrim:
        new-old>pg size : inc
        inc < 0         : dec
        0               : return cur break
    */
    if(inc < 0) {

        if(new_curr < heap->curr - inc && new_curr > old_curr){
            // underflow / invalid shrink past start
            return (void*)-1;
        }

        uintptr_t new_max = CEIL(new_curr, PAGE_SIZE);

        for (uintptr_t addr = new_max; addr < heap->max; addr += PAGE_SIZE) {
            unmap_page(addr);
        }
        
        heap->curr = new_curr;
        heap->max = new_max;
        return (void*)new_curr;
    }

    while (heap->max < new_curr) {
        uint32_t new_frame = alloc_frame();
        if (new_frame == 0) {
            debug_print("[OUT OF MEM!]\n");
            return (void*)-1;
            
        }
        map_page(heap->max, new_frame, 0x3);
        heap->max += PAGE_SIZE;
    }

    heap->curr = new_curr;
    return (void *)old_curr;
    

}