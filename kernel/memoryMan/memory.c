#include "memory.h"

void initMem(multiboot_info* boot_data){
    for(int i =0; i < boot_data->mmap_length; i+= sizeof(multiboot_mmap_entry)){
        
        multiboot_mmap_entry *indexEntry = (multiboot_mmap_entry*)(boot_data->mmap_addr + i);
        
        printf(" Addr lo: %x | Addr hi: %x | Len lo:  %x | Len hi: %x | Size: %x | Type: ",
        indexEntry->addr_lo, indexEntry->addr_hi, indexEntry->len_lo, indexEntry->len_hi, indexEntry->size);
        
        switch(indexEntry->type){
            case 1:
                printf("MULTIBOOT_MEMORY_AVAILABLE\n");
                break;
            case 2:
                printf("MULTIBOOT_MEMORY_RESERVED\n");
                break;
            case 3:
                printf("MULTIBOOT_MEMORY_ACPI_RECLAIMABLE\n");
                break;
            case 4:
                printf("MULTIBOOT_MEMORY_NVS\n");
                break;
            case 5:
                printf("MULTIBOOT_MEMORY_BADRAM\n");
                break;
            default:
                break;
        }
    }
}
