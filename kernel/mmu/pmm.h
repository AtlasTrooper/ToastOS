#include "../limine.h"
#include <stddef.h>
#include <stdint.h>
#include "../stdlib/stdio.h"

uint64_t get_hhdm();
void* get_virt_addr(uint64_t *paddr);
void* get_phys_addr(uint64_t *vaddr);
void init_pmm();