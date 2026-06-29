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

const pmm_header_t *get_pmm_header(void) { return &pmm; }
uint64_t get_hhdm(void)                  { return pmm.hhdm_base; }

void *get_virt_addr(uint64_t paddr) {
    return (void *)(paddr + pmm.hhdm_base);
}

uint64_t get_phys_addr(void *vaddr) {
    return (uint64_t)vaddr - pmm.hhdm_base;
}

struct limine_memmap_response *get_memmap(void) {
    return memmap_request.response;
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
    return 0;
}

void free_frame(uint64_t paddr) {
    uint64_t frame = paddr >> PAGE_SHIFT;
    if (frame < pmm.max_frames && BITMAP_TEST(frame)) {
        BITMAP_CLEAR(frame);
        pmm.free_frames++;
    }
}

void init_pmm(void) {
    if (hhdm_request.response == NULL) KPANIC(NULL, "HHDM BAD RESPONSE");
    pmm.hhdm_base = hhdm_request.response->offset;

    if (kaddr_request.response == NULL) KPANIC(NULL, "KADDR BAD RESPONSE");
    pmm.kernel_phys_base = kaddr_request.response->physical_base;
    pmm.kernel_virt_base = kaddr_request.response->virtual_base;

    struct limine_memmap_response *memmap = memmap_request.response;
    if (memmap == NULL) KPANIC(NULL, "MEMMAP BAD RESPONSE");

    //Kernel size calculation
    pmm.kernel_phys_end = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];
        if (ent->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            uint64_t end = ent->base + ent->length;
            if (end > pmm.kernel_phys_end) pmm.kernel_phys_end = end;
        }
    }
    if (pmm.kernel_phys_end == 0) KPANIC(NULL, "BAD KERNEL END");

    //Bitmap size calculation (Right after kernel)

    // for()

    pmm.bitmap_phys = CEIL(pmm.kernel_phys_end, PAGE_SIZE);
    pmm.b_map = (uint64_t*)get_virt_addr(pmm.bitmap_phys);

    // pmm.bitmap_bytes = CEIL()

}