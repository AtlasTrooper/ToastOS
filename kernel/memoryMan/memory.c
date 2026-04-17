#include "memory.h"

//Presents the memory regions and their types using multiboot
void initMem(multiboot_info* boot_data){
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
