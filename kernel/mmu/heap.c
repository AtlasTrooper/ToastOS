#include "heap.h"
#include "pmm.h"

extern void*  dlmalloc(size_t bytes);
extern void   dlfree(void* mem);
extern void*  dlrealloc(void* mem, size_t newsize);
extern int    dlmalloc_trim(size_t pad);

static heap_t k_heap_storage;
static heap_t *k_heap = NULL;

#define PAGE_ALIGN_UP(x) CEIL((x), PAGE_SIZE)

void* sys_sbrk(intptr_t increment) {
    heap_t *heap = k_heap;
    if (!heap) return (void*)-1;

    uint64_t old_brk = heap->brk;

    if (increment == 0) {
        return (void*)old_brk;
    }

    if (increment > 0) {
        uint64_t new_brk = old_brk + (uint64_t)increment;
        if (new_brk > heap->limit) {
            // Would exceed the heap's reserved virtual range.
            return (void*)-1;
        }

        uint64_t need_top = PAGE_ALIGN_UP(new_brk);
        uint64_t mapped_so_far = heap->mapped_end;

        while (mapped_so_far < need_top) {
            uint64_t phys = pmm_alloc();
            if (!phys) {
                heap->mapped_end = mapped_so_far;
                return (void*)-1;
            }
            vmm_map_page(heap->vmm_ctx, mapped_so_far, phys, heap->flags);
            mapped_so_far += PAGE_SIZE;
        }

        heap->mapped_end = mapped_so_far;
        heap->brk = new_brk;
        return (void*)old_brk;
    }

    // --- Shrinking ---
    uint64_t shrink_by = (uint64_t)(-increment);
    uint64_t new_brk = (shrink_by > (old_brk - heap->base_addr))
                            ? heap->base_addr
                            : old_brk - shrink_by;

    uint64_t keep_top = PAGE_ALIGN_UP(new_brk);
    uint64_t mapped_top = heap->mapped_end;

    while (mapped_top > keep_top) {
        mapped_top -= PAGE_SIZE;
        uint64_t phys = vmm_get_phys(heap->vmm_ctx, mapped_top);
        if (phys) pmm_free(phys);
        vmm_unmap_page(heap->vmm_ctx, mapped_top);
    }

    heap->mapped_end = mapped_top;
    heap->brk = new_brk;
    return (void*)old_brk;
}

/*
 Dedicated Kheap region to prevent us from
 walking into the hhdm memory territory
*/
#define KHEAP_VIRT_BASE 0xFFFF900000000000ULL

void init_kheap(void) {
    k_heap = &k_heap_storage;
    k_heap->base_addr  = KHEAP_VIRT_BASE;
    k_heap->brk        = KHEAP_VIRT_BASE;
    k_heap->mapped_end = KHEAP_VIRT_BASE;
    k_heap->limit      = KHEAP_VIRT_BASE + (1024ULL * 1024 * 64);
    k_heap->flags      = VMM_FLAGS_KERNEL_RW;
    k_heap->vmm_ctx    = get_current_context();

    debug_print_hex("Kernel heap base:  ", k_heap->base_addr);
    debug_print_hex("Kernel heap limit: ", k_heap->limit);

}

heap_t* k_heap_status(void) {
    return k_heap;
}

int heap_is_valid(const heap_t *heap) {
    return heap != NULL && heap->vmm_ctx != NULL;
}

void* kmalloc(size_t size) {
    if (!k_heap) return NULL;
    return dlmalloc(size);
}

void kfree(void *ptr) {
    if (!k_heap || !ptr) return;
    dlfree(ptr);
}

void* krealloc(void *ptr, size_t size) {
    if (!k_heap) return NULL;
    return dlrealloc(ptr, size);
}

int kheap_trim(size_t pad) {
    if (!k_heap) return 0;
    return dlmalloc_trim(pad);
}

#include "dlmalloc.c"