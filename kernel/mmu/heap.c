#include "heap.h"
#include "dlmalloc_config.h"   /* must come before dlmalloc.c is included */
#include "dlmalloc.c"          /* compile dlmalloc as part of this TU     */

/* ═══════════════════════════════════════════════════════════════════════════
 * heap_t  –  low-level sbrk-style engine
 * ═══════════════════════════════════════════════════════════════════════════ */

void heap_init(heap_t *heap, uintptr_t base) {
    heap->base    = base;
    heap->curr    = base;
    heap->mapped  = base;   /* nothing mapped yet */
    heap->is_init = 1;
}

/*
 * heap_sbrk
 *
 * inc > 0 : extend the break, map new pages as needed.
 *           Returns the OLD break (dlmalloc uses the returned pointer as the
 *           start of the new usable region, which is exactly old break).
 *
 * inc < 0 : shrink the break, unmap now-unused pages.
 *           Returns the NEW break on success, (void*)-1 on underflow.
 *
 * inc = 0 : return current break unchanged.
 */
void *heap_sbrk(heap_t *heap, int64_t inc) {
    if (!heap || !heap->is_init) return (void *)-1;

    if (inc == 0) return (void *)heap->curr;

    if (inc > 0) {
        uintptr_t old_curr = heap->curr;
        uintptr_t new_curr = heap->curr + (uint64_t)inc;

        while (heap->mapped < new_curr) {
            uint64_t frame = alloc_frame();
            if (frame == 0) {
                /* OOM — back out: unmap what we mapped this call */
                for (uintptr_t a = old_curr; a < heap->mapped; a += PAGE_SIZE)
                    unmap_page(a);
                heap->mapped = old_curr;
                return (void *)-1;
            }
            map_page(heap->mapped, frame, VMM_FLAGS_KERNEL_RW);
            heap->mapped += PAGE_SIZE;
        }

        heap->curr = new_curr;
        return (void *)old_curr;   /* dlmalloc expects the OLD break */
    }

    uint64_t  decrement = (uint64_t)(-inc);

    if (decrement > (heap->curr - heap->base)) {
        return (void *)-1;
    }

    uintptr_t new_curr   = heap->curr - decrement;
    uintptr_t new_mapped = CEIL(new_curr, PAGE_SIZE);

    /* Unmap pages that are now fully above the new break */
    for (uintptr_t addr = new_mapped; addr < heap->mapped; addr += PAGE_SIZE)
        unmap_page(addr);

    heap->curr   = new_curr;
    heap->mapped = new_mapped;
    return (void *)new_curr;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Kernel heap  –  one global mspace built on top of the kernel heap_t
 * ═══════════════════════════════════════════════════════════════════════════ */

static heap_t  k_heap;
static mspace  k_mspace;

/*
 * k_mspace_more  –  dlmalloc calls this when the mspace needs more memory.
 *
 * dlmalloc passes `size` (already page-rounded by it) and expects us to
 * return a pointer to `size` bytes of usable address space, or MFAIL on OOM.
 * We simply grow the kernel heap_t by that amount.
 */
static void *k_mspace_more(mspace sp, size_t size) {
    (void)sp;   /* we only have one kernel mspace */
    void *ret = heap_sbrk(&k_heap, (int64_t)size);
    return (ret == (void *)-1) ? MFAIL : ret;
}

void init_kheap(void) {
    /*
     * Place the heap immediately after the PMM bitmap.
     * alloc_start_frame is the first allocatable frame; the bitmap ends
     * just before it.  We add the HHDM offset to get a virtual address.
     */
    const pmm_header_t *pmm  = get_pmm_header();
    uintptr_t           base = (uintptr_t)get_virt_addr(
                                    pmm->alloc_start_frame << PAGE_SHIFT);

    heap_init(&k_heap, base);

    /*
     * Seed the mspace with an initial chunk so dlmalloc has something to
     * work with immediately.  We give it one page; it will call k_mspace_more
     * for anything larger.
     *
     * create_mspace_with_base(base, size, locked)
     *   base  – pointer to pre-allocated memory dlmalloc can use right away
     *   size  – how many bytes at `base` are available
     *   locked – 0 (we set USE_LOCKS 0)
     */
    void *seed = heap_sbrk(&k_heap, PAGE_SIZE);
    if (seed == (void *)-1) {
        KPANIC(NULL, "init_kheap: failed to allocate initial page");
    }

    k_mspace = create_mspace_with_base(seed, PAGE_SIZE, 0);
    if (!k_mspace) {
        KPANIC(NULL, "init_kheap: create_mspace_with_base failed");
    }

    /*
     * Register our grow callback so dlmalloc can extend the heap
     * automatically.
     */
    mspace_set_more(k_mspace, k_mspace_more);
}

void *kmalloc(size_t size) {
    return mspace_malloc(k_mspace, size);
}

void kfree(void *ptr) {
    mspace_free(k_mspace, ptr);
}

void *krealloc(void *ptr, size_t size) {
    return mspace_realloc(k_mspace, ptr, size);
}

void *kcalloc(size_t nmemb, size_t size) {
    return mspace_calloc(k_mspace, nmemb, size);
}

heap_t *k_heap_status(void) {
    return k_heap.is_init ? &k_heap : NULL;
}
