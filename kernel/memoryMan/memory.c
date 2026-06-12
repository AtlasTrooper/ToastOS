#include "memory.h"

#define PAGE_SIZE 4096
#define ALIGN 4096

void *memset(void *dest, int val, unsigned int iter){
    unsigned char* ptr =dest;
    while(iter--){
        *ptr ++ = (unsigned char) val;
    }
    return dest;
}

static uintptr_t v_addr_s = (uintptr_t) &_kernel_v_start;
static uintptr_t v_addr_e = (uintptr_t) &_kernel_v_end;
static uintptr_t p_addr_s = (uintptr_t) &_kernel_p_start;
static uintptr_t p_addr_e = (uintptr_t) &_kernel_p_end;

static uint32_t* pd_v_addr = (uint32_t*)(0xFFFFF000);
static uintptr_t p_alloc_s;
static uintptr_t k_heap_curr = 0;
static uintptr_t k_heap_max = 0;

void initPmm(multiboot_info* boot_data){
    

    multiboot_info *mboot = (multiboot_info *)((uintptr_t)boot_data + KERNEL_START);
    max_frames = (mboot->mem_upper + 1024)/4; //in 4KB
    uint32_t chunk_count = max_frames/32; //uint32 contains 32 4kb frame trackers
    b_map = (uint32_t*)(v_addr_e);

    memset(b_map, 0, chunk_count*sizeof(uint32_t));

    uintptr_t p_bitmap_e = p_addr_e + (chunk_count * sizeof(uint32_t));
    p_alloc_s = CEIL(p_bitmap_e, ALIGN);

    uint32_t used_frames = p_alloc_s/ PAGE_SIZE;
    
    for(uint32_t i = 0; i < used_frames; i++){
        BITMAP_SET(i);
    }

    page_directory[0] = 0;
    invalidate_page(0);
    page_directory[1023] = ((uint32_t) page_directory - KERNEL_START) | 0x3;
    invalidate_page(0xFFFFF000);

    for(uint32_t i = (uint32_t)p_addr_s; i < (uint32_t)p_alloc_s; i+=PAGE_SIZE){
        map_page(i+KERNEL_START, i, 0x3);
    }

    uint32_t p_pd_addr = (uint32_t)(page_directory)-KERNEL_START;

    reload_CR3(p_pd_addr);
    
    //initialize the heap
    init_Kheap(p_alloc_s);
}

uint32_t alloc_frame(){
    for(uint32_t i = 0 ; i<(max_frames/32); i++){
        if(b_map[i] != 0xFFFFFFFF){
            uint32_t pos = i*32;
            for(int j = 0; j< 32; j++){
                if (pos+j > max_frames){
                    return 0;
                }

                if(!BITMAP_TEST(pos + j)){
                    BITMAP_SET(pos + j);
                    return (uint32_t)((pos+j)*PAGE_SIZE);
                }
            }
        }
    }
    return 0; //we out
}

void free_frame(uint32_t p_addr){
    BITMAP_CLEAR((uint32_t)(p_addr/PAGE_SIZE));
}

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
    pd_v_addr[pt_index] = 0;
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

void init_Kheap(uintptr_t heap_s) {
    k_heap_curr = heap_s;
    k_heap_max = CEIL(heap_s, PAGE_SIZE);
}

void *heapafus(int32_t inc) {
    uintptr_t old_curr = k_heap_curr;
    uintptr_t new_curr = k_heap_curr + inc;

    /*
    Nechalek lemikrim:
        new-old>pg size : inc
        inc < 0         : dec
        0               : return cur break
    */
    if(inc < 0) {

        if(new_curr < k_heap_curr - inc && new_curr > old_curr){
            // underflow / invalid shrink past start
            return (void*)-1;
        }

        uintptr_t new_max = CEIL(new_curr, PAGE_SIZE);

        for (uintptr_t addr = new_max; addr < k_heap_max; addr += PAGE_SIZE) {
            unmap_page(addr);
        }
        
        k_heap_curr = new_curr;
        k_heap_max = new_max;
        return (void*)new_curr;
    }

    while (k_heap_max < new_curr) {
        uint32_t new_frame = alloc_frame();
        if (new_frame == 0) {
            debug_print("[OUT OF MEM!]\n");
            return (void*)-1;
            
        }
        map_page(k_heap_max, new_frame, 0x3);
        k_heap_max += PAGE_SIZE;
    }

    k_heap_curr = new_curr;
    return (void *)old_curr;
    

}