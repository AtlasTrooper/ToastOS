#include "vmm.h"

void invalidate_page(uint32_t addr){
    asm volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

void map_page(uint32_t v_addr, uint32_t p_addr, uint32_t pdt_flags){    
    uint32_t pd_index = v_addr >> 22;
    uint32_t pt_index = ((v_addr >> 12) & 0x03FF);

    uint32_t* pt_v_addr = (uint32_t*)(0xFFC00000) + (0x400 * pd_index);
    

    if(!(pd_v_addr[pd_index] & 0x1)){
        uint32_t new_pt_p_addr = alloc_frame();

        if(new_pt_p_addr == 0) {
            debug_print("Out of MEM! Leaving page unmapped\n");
            return;
        }

        pd_v_addr[pd_index] = new_pt_p_addr | pdt_flags;

        invalidate_page((uint32_t)pt_v_addr);

        memset(pt_v_addr, 0, PAGE_SIZE);
    }

    pt_v_addr[pt_index] = p_addr | pdt_flags;
    invalidate_page((uint32_t)v_addr);
}

void unmap_page(uint32_t v_addr) {
    uint32_t pd_index = v_addr >> 22;
    uint32_t pt_index = ((v_addr >> 12) & 0x03FF);
    uint32_t* pt_v_addr = (uint32_t*)(0xFFC00000) + (0x400 * pd_index);

    if(!(pd_v_addr[pd_index] & 0x1)) {
        return;
    }

    if(!(pt_v_addr[pt_index] & 0x1)) {
        return;
    }
    uint32_t p_addr = pt_v_addr[pt_index] & ~(0xFFF);
    free_frame(p_addr);
    pt_v_addr[pt_index] = 0;
    invalidate_page((uint32_t)v_addr);    
}


void reload_CR3(uint32_t p_pd_addr){
    asm volatile(
        "mov %0, %%cr3\n"
        :
        :"r"(p_pd_addr)
        :"memory"
    ); //reload CR3
}

uint32_t* getCurrPDT() {
    uint32_t* addr;
    asm volatile ("mov %0, %%cr3" : "=r" (addr));
    return (uint32_t*)(addr+KERNEL_START);
}

void syncPDT() {
    return;
}

void changePDT(uint32_t pdt) {
    reload_CR3(pdt);
}
