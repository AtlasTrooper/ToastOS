#include "memory.h"

/*This is Outdated, use new paging based initMem
//Presents the memory regions and their types using multiboot
void multiboot_mmap(multiboot_info* boot_data){
    for(int i =0; i < boot_data->mmap_length; i+= sizeof(multiboot_mmap_entry)){
        
        multiboot_mmap_entry *indexEntry = (multiboot_mmap_entry*)(boot_data->mmap_addr + i);
        
        printf(" Addr lo: %x | Addr hi: %x | Len lo:  %x | Len hi: %x | Size: %x |\n |Type: ",
        indexEntry->addr_lo, indexEntry->addr_hi, indexEntry->len_lo, indexEntry->len_hi, indexEntry->size);
        
        switch(indexEntry->type){
            case 1:
                printf("MULTIBOOT_MEMORY_AVAILABLE");
                break;
            case 2:
                printf("MULTIBOOT_MEMORY_RESERVED");
                break;
            case 3:
                printf("MULTIBOOT_MEMORY_ACPI_RECLAIMABLE");
                break;
            case 4:
                printf("MULTIBOOT_MEMORY_NVS");
                break;
            case 5:
                printf("MULTIBOOT_MEMORY_BADRAM");
                break;
            default:
                break;
        }
        printf("|\n");
    }
}
*/

//uint32_t* b_map;
//uint32_t max_frames;

void *memset(void *dest, int val, unsigned int iter){
    unsigned char* ptr =dest;
    while(iter--){
        *ptr ++ = (unsigned char) val;
    }
    return dest;
}

void initMem(multiboot_info* boot_data){
    uintptr_t v_addr_s = (uintptr_t) &_kernel_v_start;
    uintptr_t v_addr_e = (uintptr_t) &_kernel_v_end;

    uintptr_t p_addr_s = (uintptr_t) &_kernel_p_start;
    uintptr_t p_addr_e = (uintptr_t) &_kernel_p_end;

    multiboot_info *mboot = (multiboot_info *)((uintptr_t)boot_data + KERNEL_START);
    max_frames = (mboot->mem_upper + 1024)/4; //in 4KB
    uint32_t chunk_count = max_frames/32; //uint32 contains 32 4kb frame trackers

    b_map = (uint32_t*)(v_addr_e);

    memset(b_map, 0, chunk_count*sizeof(uint32_t));

    uintptr_t p_bitmap_e = p_addr_e + (chunk_count * sizeof(uint32_t));
    uintptr_t p_alloc_s = CEIL(p_bitmap_e, 4096);

    uint32_t used_frames = p_alloc_s/ PAGE_SIZE;
    

    printf(" Initializing memory map...\n\n Virtual start: %p |\n Virtual end: %p |\n Physical start: %p |\n Physical end: %p |\n\n",
         v_addr_s, v_addr_e, p_addr_s, p_addr_e);

    
    for(uint32_t i = 0; i < used_frames; i++){
        BITMAP_SET(i);
    }

    printf("[PAGE FRAME ALLOCATION INITIALIZED]: Tracking %d frames. \n", max_frames);
    printf("Bitmap positioned at: %p\n", b_map);
    printf("First available physical frame: %p \n\n", p_alloc_s);

    printf("Reconfiguring page tables... setting up 4KB pages\n");

    
    page_directory[1023] = ((uint32_t) page_directory - KERNEL_START) | 0x3;
    //printf("%d LAST STOP\n", page_directory[1023]);
    invalidate_page(0xFFFFF000);

    printf("[Starting mapping sequence!]\n");

    for(uint32_t i = (uint32_t)p_addr_s; i < (uint32_t)p_bitmap_e; i+=PAGE_SIZE){
        map_page(i+KERNEL_START, i, 0x3);
    }

    uint32_t p_pd_addr = (uint32_t)&(page_directory)-KERNEL_START;

    reload_CR3(p_pd_addr);

    printf("[Mapping sequence complete!]\n");
    
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
    /*
    to do:
    1. check if PT is occupied here
    2. get PT v_addr
    3. map physical frame and flags into table
    4. flush tlb
    */
    
    uint32_t pd_index = v_addr >> 22;
    uint32_t pt_index = ((v_addr >> 12) & 0x03FF);

    uint32_t* pt_v_addr = (uint32_t*)(0xFFC00000) + (0x400 * pd_index);
    uint32_t* pd_v_addr = (uint32_t*)(0xFFFFF000);

    if(!(pd_v_addr[pd_index] & 0x1)){
        uint32_t new_pt_p_addr = alloc_frame();
        printf("NEW PT AT %p \n", new_pt_p_addr);
        pd_v_addr[pd_index] = new_pt_p_addr | pdt_flags;

        invalidate_page(*pt_v_addr);

        memset(pt_v_addr, 0, 4096);
    }

    //uint32_t *pt = (uint32_t *)((page_directory[pd_index] & ~0xFFF)+KERNEL_START);
    pt_v_addr[pt_index] = p_addr | pdt_flags;
    
    debug_print("Mapped page\n");
}

void reload_CR3(uint32_t p_pd_addr){
    asm volatile(
        "mov %0, %%cr3\n"
        :
        :"r"(p_pd_addr)
        :"memory"
    ); //reload CR3
}
