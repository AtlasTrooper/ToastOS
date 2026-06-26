#include "pmm.h"

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};
static volatile struct limine_kernel_address_request kaddr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static pmm_header_t pmm;

const pmm_header_t *get_pmm_header(void) {
    return &pmm;
}

uint64_t get_hhdm(void) {
    return pmm.hhdm_base;
}

void *get_virt_addr(uint64_t paddr) {
    return (void *)(paddr + pmm.hhdm_base);
}

uint64_t get_phys_addr(void *vaddr) {
    return (uint64_t)vaddr - pmm.hhdm_base;
}

uint64_t alloc_frame(void) {
    uint64_t start_word = pmm.alloc_start_frame >> 6;
    uint64_t words      = (pmm.max_frames + 63) >> 6;

    for (uint64_t i = start_word; i < words; i++) {
        if (pmm.b_map[i] == 0xFFFFFFFFFFFFFFFFULL) continue;

        int      j     = __builtin_ctzll(~pmm.b_map[i]);
        uint64_t frame = (i << 6) | j;

        if (frame >= pmm.max_frames) return 0;

        BITMAP_SET(frame);
        pmm.free_frames--;
        return frame << PAGE_SHIFT;
    }
    return 0; /* OOM */
}

void free_frame(uint64_t paddr) {
    uint64_t frame = paddr >> PAGE_SHIFT;
    if (BITMAP_TEST(frame)) {
        BITMAP_CLEAR(frame);
        pmm.free_frames++;
    }
}

void init_pmm(void) {
    if (hhdm_request.response == NULL) return; /* TODO: panic */
    pmm.hhdm_base = hhdm_request.response->offset;

    if (kaddr_request.response == NULL) return; /* TODO: panic */
    pmm.kernel_phys_base = kaddr_request.response->physical_base;
    pmm.kernel_virt_base = kaddr_request.response->virtual_base;

    struct limine_memmap_response *memmap = memmap_request.response;
    if (memmap == NULL) return; /* TODO: panic */

    pmm.kernel_phys_end = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];
        if (ent->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            uint64_t end = ent->base + ent->length;
            if (end > pmm.kernel_phys_end) pmm.kernel_phys_end = end;
        }
    }
    if (pmm.kernel_phys_end == 0) return; /* TODO: panic */

    uint64_t highest = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];
        if (ent->type != LIMINE_MEMMAP_USABLE) continue;
        uint64_t top = ent->base + ent->length;
        if (top > highest) highest = top;
    }

    pmm.max_frames   = highest >> PAGE_SHIFT;
    pmm.bitmap_phys  = CEIL(pmm.kernel_phys_end, PAGE_SIZE);
    pmm.bitmap_bytes = CEIL((pmm.max_frames + 63) >> 3, PAGE_SIZE);

    pmm.b_map = (uint64_t *)get_virt_addr(pmm.bitmap_phys);
    memset(pmm.b_map, 0xFF, pmm.bitmap_bytes);

    uint64_t alloc_phys      = CEIL(pmm.bitmap_phys + pmm.bitmap_bytes, PAGE_SIZE);
    pmm.alloc_start_frame    = alloc_phys >> PAGE_SHIFT;
    pmm.free_frames          = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];
        if (ent->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t frame       = ent->base >> PAGE_SHIFT;
        uint64_t frame_count = ent->length >> PAGE_SHIFT;

        if (frame < pmm.alloc_start_frame) {
            uint64_t skip = pmm.alloc_start_frame - frame;
            if (skip >= frame_count) continue;
            frame       += skip;
            frame_count -= skip;
        }

        pmm.free_frames += frame_count;

        uint64_t word = frame >> 6;
        uint64_t bit  = frame & 63;

        if (bit != 0) {
            uint64_t count = 64 - bit;
            if (count > frame_count) count = frame_count;
            uint64_t mask = ((count < 64) ? (1ULL << count) - 1ULL : ~0ULL) << bit;
            pmm.b_map[word++] &= ~mask;
            frame_count       -= count;
        }

        uint64_t full_words = frame_count >> 6;
        memset(&pmm.b_map[word], 0x00, full_words * sizeof(uint64_t));
        word        += full_words;
        frame_count &= 63;

        if (frame_count != 0) {
            uint64_t mask = (1ULL << frame_count) - 1ULL;
            pmm.b_map[word] &= ~mask;
        }
    }
}