#include "heap.h"

extern mspace create_mspace(size_t capacity, int locked);
extern void* mspace_malloc(mspace msp, size_t bytes);
extern void mspace_free(mspace msp, void* mem);
extern void* mspace_realloc(mspace msp, void* mem, size_t newsize);

static heap_t *k_heap = NULL;
static heap_t *active_allocation_heap = NULL;


void* sys_mmap_alloc(size_t size) {
    heap_t *heap = active_allocation_heap;
    if (!heap) heap = k_heap;
    if (!heap) return (void*)-1; // CMFAIL / MAP_FAILED

    if (heap->end_addr + size > heap->limit) return (void*)-1;

    uint64_t start_addr = heap->end_addr;
    uint64_t end_addr = start_addr + size;

    uint64_t start_page = CEIL(start_addr, PAGE_SIZE);
    uint64_t end_page = CEIL(end_addr, PAGE_SIZE);

    for (uint64_t addr = start_page; addr < end_page; addr += PAGE_SIZE) {
        uint64_t phys = pmm_alloc();
        if (!phys) return (void*)-1;
        vmm_map_page(heap->vmm_ctx, addr, phys, heap->flags);
    }

    heap->end_addr = end_addr;
    return (void*)start_addr;
}

int sys_munmap_free(void* addr, size_t size) {
    heap_t *heap = active_allocation_heap;
    if (!heap) heap = k_heap;
    if (!heap || !addr || size == 0) return -1;

    uint64_t start_addr = (uint64_t)addr;
    uint64_t end_addr   = start_addr + size;

    uint64_t start_page = start_addr & PAGE_MASK;
    uint64_t end_page   = CEIL(end_addr, PAGE_SIZE);

    for (uint64_t p_addr = start_page; p_addr < end_page; p_addr += PAGE_SIZE) {
        uint64_t phys = vmm_get_phys(heap->vmm_ctx, p_addr);
        if (phys) {
            pmm_free(phys);
        }
        vmm_unmap_page(heap->vmm_ctx, p_addr);
    }

    return 0;
}

heap_t* heap_create(
            vmm_context_t *vmm_ctx,
            uint64_t base,
            uint64_t initial_size,
            uint64_t max_size,
            uint64_t flags) {

    initial_size = CEIL(initial_size, PAGE_SIZE);
    heap_t *heap = NULL;

    if (k_heap == NULL) {
        static heap_t meta_heap;
        k_heap = &meta_heap;
        heap = k_heap;
    } else {
        heap = (heap_t*)kmalloc(sizeof(heap_t));
        if (!heap) return NULL;
    }

    heap->vmm_ctx = vmm_ctx;
    heap->base_addr = base;
    heap->end_addr = base;
    heap->limit = base + max_size;
    heap->flags = flags;

    debug_print_hex("Heap flags: ", flags);

    heap_t *prev_active = active_allocation_heap;
    active_allocation_heap = heap;

    mspace ms = create_mspace(0, 0);
    
    active_allocation_heap = prev_active;

    if (!ms) {
        if (heap != k_heap) kfree(heap);
        return NULL;
    }

    heap->ms = ms;
    return heap;
}

void* heap_alloc(heap_t *heap, size_t bytes) {
    if (!heap) return NULL;
    
    heap_t *prev_active = active_allocation_heap;
    active_allocation_heap = heap;

    void *ptr = mspace_malloc(heap->ms, bytes);

    active_allocation_heap = prev_active;
    return ptr;
}

void heap_free(heap_t *heap, void *ptr) {
    if (!heap || !ptr) return;
    mspace_free(heap->ms, ptr);
}

void* heap_realloc(heap_t *heap, void *ptr, size_t new_bytes) {
    if (!heap) return NULL;

    heap_t *prev_active = active_allocation_heap;
    active_allocation_heap = heap;

    void *new_ptr = mspace_realloc(heap->ms, ptr, new_bytes);

    active_allocation_heap = prev_active;
    return new_ptr;
}

void init_kheap() {
    uint64_t highest = get_max_addr();
    uint64_t alligned_high = CEIL(highest, PAGE_SIZE);
    uint64_t base = alligned_high + get_hhdm();

    if (!heap_create(get_current_context(), base, 1024 * 1024 * 16, 1024 * 1024 * 64, (PTE_WRITE | PTE_PRESENT))) {
        KPANIC(NULL, "init_kheap: failed to instantiate kernel heap framework.");
    }
}

int heap_is_valid(const heap_t *heap) {
    return (heap != NULL && heap->ms != NULL);
}

heap_t* k_heap_status(void) {
    return k_heap;
}

void* kmalloc(size_t size) { return heap_alloc(k_heap, size); }
void  kfree(void *ptr)     { heap_free(k_heap, ptr); }
void* krealloc(void *ptr, size_t size) { return heap_realloc(k_heap, ptr, size); }

#include "dlmalloc.c"