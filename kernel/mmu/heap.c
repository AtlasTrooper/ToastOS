#include "heap.h"
#include "dlmalloc_config.h"
#include "dlmalloc.c"

extern mspace create_mspace_with_base(void* base, size_t capacity, int locked);
extern void* mspace_malloc(mspace msp, size_t bytes);
extern void mspace_free(mspace msp, void* mem);
extern void* mspace_realloc(mspace msp, void* mem, size_t newsize);
extern size_t mspace_add_to_space(mspace msp, char* amt, size_t len);

static heap_t *k_heap = NULL;

heap_t* heap_create(
            vmm_context_t *vmm_ctx,
            uint64_t base,
            uint64_t initial_size,
            uint64_t max_size,
            uint64_t flags) {

                initial_size = CEIL(initial_size, PAGE_SIZE);
                
                for(uint64_t offset = 0; offset < initial_size; offset += PAGE_SIZE) {
                    uint64_t phys = pmm_alloc();
                    vmm_map_page(vmm_ctx, base + offset, phys, flags);
                }

                mspace ms = create_mspace_with_base((void*)base, initial_size, 0);
                if(!ms) return NULL; //should I KPANIC here?

                heap_t *heap = (heap_t*)mspace_malloc(ms, sizeof(heap_t));
                if(!heap) return NULL;

                heap->ms = ms;
                heap->base_addr = base;
                heap->end_addr = base + initial_size;
                heap->limit = base + max_size;
                heap->flags = flags;
                heap->vmm_ctx = vmm_ctx;

                return heap;
            }

void* heap_alloc(heap_t *heap, size_t bytes) {
    if (!heap) return NULL;

    void *ptr = mspace_malloc(heap->ms, bytes);

    if (!ptr && (heap->end_addr < heap->limit)) {

        uint64_t inc = CEIL(bytes, PAGE_SIZE);
        
        if (heap->end_addr + inc > heap->limit) {
            inc = heap->limit - heap->end_addr;
        }

        if (inc < PAGE_SIZE) return NULL;

        for (uint64_t addr = heap->end_addr; addr < heap->end_addr + inc; addr += PAGE_SIZE) {
            uint64_t phys = pmm_alloc();
            vmm_map_page(heap->vmm_ctx, addr, phys, heap->flags);
        }

        mspace_add_to_space(heap->ms, (char*)heap->end_addr, inc);
        
        heap->end_addr += inc;
        ptr = mspace_malloc(heap->ms, bytes);
    }

    return ptr;
}

void heap_free(heap_t *heap, void *ptr) {
    if (!heap || !ptr) return;
    mspace_free(heap->ms, ptr);
}

void* heap_realloc(heap_t *heap, void *ptr, size_t new_bytes) {
    if (!heap) return NULL;
    return mspace_realloc(heap->ms, ptr, new_bytes);
}

void init_kheap() {
    uint64_t highest = get_max_addr();
    uint64_t alligned_high = CEIL(highest, PAGE_SIZE);
    uint64_t base = alligned_high + get_hhdm();

    k_heap = heap_create(
        get_current_context(),
        base,
        1024 * 1024 * 16,
        1024 * 1024 * 64,
        PTE_WRITE
    );

    if (!k_heap) {
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