#include "pmm.h"

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
    
    //initialize the kernel heap at the first higher half allocatable and page aligned addr
    init_Kheap(p_alloc_s + KERNEL_START);
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
