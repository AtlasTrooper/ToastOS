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
uint64_t get_max_addr(void)                   { return pmm.max_frames * PAGE_SIZE;}
void *get_virt_addr(uint64_t paddr) {
    return (void *)(paddr + pmm.hhdm_base);
}

uint64_t get_phys_addr(void *vaddr) {
    return (uint64_t)vaddr - pmm.hhdm_base;
}

struct limine_memmap_response *get_memmap(void) {
    return memmap_request.response;
}

static void free_frame(uint64_t frame_addr) {
    uint64_t frame = frame_addr / 4096;
    uint64_t byte_idx = frame / 8;
    uint64_t bit_idx = frame % 8;

    pmm.b_map[byte_idx] &= ~(1 << bit_idx);
    pmm.free_frames++;
}

static void alloc_frame(uint64_t frame_addr) {
    uint64_t frame = frame_addr / 4096;
    uint64_t byte_idx = frame / 8;
    uint64_t bit_idx = frame % 8;

    pmm.b_map[byte_idx] |= (1 << bit_idx);
    pmm.free_frames < 0 ? pmm.free_frames = 0 : pmm.free_frames --;
}

uint64_t pmm_alloc(void) {
   for (size_t i = 0; i < pmm.bitmap_bytes; i++) {
        if (pmm.b_map[i] == 0xFF) continue; 

        for (int bit = 0; bit < 8; bit++) {
            if (!(pmm.b_map[i] & (1 << bit))) {
                uint64_t frame_idx = (i * 8) + bit;
                uint64_t phys_addr = frame_idx * 4096;
                
                alloc_frame(phys_addr);
                return phys_addr;
            }
        }
    }
    return 0; // Out of memory!
}

void pmm_free(uint64_t paddr) {
    free_frame(paddr);
}

void init_pmm(void) {
    if (hhdm_request.response == NULL) KPANIC(NULL, "HHDM BAD RESPONSE");
    pmm.hhdm_base = hhdm_request.response->offset;

    if (kaddr_request.response == NULL) KPANIC(NULL, "KADDR BAD RESPONSE");
    pmm.kernel_phys_base = kaddr_request.response->physical_base;
    pmm.kernel_virt_base = kaddr_request.response->virtual_base;

    struct limine_memmap_response *memmap = memmap_request.response;
    if (memmap == NULL) KPANIC(NULL, "MEMMAP BAD RESPONSE");

    uint64_t max_mem_addr = memmap->entries[0]->base;

    //Kernel size calculation
    pmm.kernel_phys_end = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];

        if (ent->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            uint64_t end = ent->base + ent->length;
            if (end > pmm.kernel_phys_end) pmm.kernel_phys_end = end;
        }

        if (ent->type == LIMINE_MEMMAP_USABLE ||
             ent->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
                uint64_t top_of_ent = ent->base + ent->length;
                if (top_of_ent > max_mem_addr) {
                    max_mem_addr = top_of_ent;
                }
        }
    }
    
    if (pmm.kernel_phys_end == 0) KPANIC(NULL, "BAD KERNEL END");

    pmm.max_frames = max_mem_addr / PAGE_SIZE;
    pmm.bitmap_bytes = pmm.max_frames/8;

    //Bitmap positioning
    pmm.b_map = NULL;

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];
        if (ent->type == LIMINE_MEMMAP_USABLE && ent->length >= pmm.bitmap_bytes) {
            pmm.b_map = (uint8_t*)(ent->base + hhdm_request.response->offset);
            pmm.bitmap_phys = ((uint64_t)pmm.b_map) - pmm.hhdm_base;
            ent->base += pmm.bitmap_bytes;
            ent->length -= pmm.bitmap_bytes;
            break;
        }
    }

    //Zero out memory
    for (size_t i = 0; i < pmm.bitmap_bytes; i++) {
        pmm.b_map[i] = 0xFF;
    }

    uint64_t first_frame_found = 0;
    int set_first_frame = 0;

    //free usable memory
     for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *ent = memmap->entries[i];
        if (ent->type == LIMINE_MEMMAP_USABLE) {
           uint64_t start_addr = (ent->base + PAGE_SIZE - 1) & PAGE_MASK;
           uint64_t end_addr = (ent->base + ent->length) & PAGE_MASK;

            if (start_addr >= end_addr) continue;

           if (!set_first_frame) {
                pmm.alloc_start_frame = ent->base;
                set_first_frame = 1;
           }
           for(uint64_t addr = start_addr; addr < end_addr; addr += PAGE_SIZE) {
                free_frame(addr);
           }
        }
    }

    

}