#include "pmm.h"


static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static uint64_t hhdm_base;

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

    if (hhdm_request.response == NULL) {
        //maybe I'll raise a page fault?
    }
    hhdm_base = hhdm_request.response -> offset;
    
    
}