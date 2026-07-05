#pragma once
#include <stdint.h>
#include <stddef.h>
#include "memory.h"
#include "pmm.h"

#define PTE_PRESENT   (1ULL <<  0)
#define PTE_WRITE     (1ULL <<  1)
#define PTE_USER      (1ULL <<  2)

#define VMM_FLAGS_KERNEL_RW   (VMM_PRESENT | VMM_WRITE)
#define VMM_FLAGS_KERNEL_RO   (VMM_PRESENT)
#define VMM_FLAGS_USER_RW     (VMM_PRESENT | VMM_WRITE | VMM_USER)
#define VMM_FLAGS_USER_RO     (VMM_PRESENT | VMM_USER)

#define PTE_FRAME_MASK 0x000FFFFFFFFFF000ULL

typedef struct vmm_context_t{
    uint64_t *pml_virt;
    uint64_t pml_phys;
} vmm_context_t;

void vmm_init(uint64_t *limine_pml4_virt);
vmm_context_t* get_current_context(void);
void vmm_switch_context(vmm_context_t *new_ctx);
void vmm_map_page(vmm_context_t *ctx,    uint64_t v_addr, uint64_t p_addr);