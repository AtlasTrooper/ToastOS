#include "pmm.h"

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static uint64_t hhdm_base; //EQUIVALENT OF KERNEL START ~2GB

uint64_t get_hhdm() {
    if (hhdm_base == NULL) return 0;
    return hhdm_base;
}

void* get_virt_addr(uint64_t *paddr) {
    return (void*)(paddr + hhdm_base);
}
void* get_phys_addr(uint64_t *vaddr) {
    return (void*)(vaddr - hhdm_base);
}



void init_pmm() {

    if (hhdm_request.response == NULL) { return; }// add panic
    hhdm_base = hhdm_request.response -> offset;

    struct limine_memmap_response *memmap = limine_memmap_request.response;

    if (memmap == NULL) {return;} // add panic

    uint64_t memmap_max = 0;
    uint64_t used_frames = 0;

    for (int i = 0; i < memmap->entry_count; i++) {
        if 
    }


    
    
}