#include "vmm.h"

static uint64_t g_hhdm_off = 0;
static vmm_context_t g_kernel_ctx;

void vmm_init(uint64_t *limine_pml4_virt) {
    g_hhdm_off = get_hhdm();
    uint64_t new_pml4_phys = pmm_alloc();
    uint64_t *new_pml4_virt = (uint64_t*)get_virt_addr(new_pml4_phys);

    memset(new_pml4_virt, 0, PAGE_SIZE);

    for (int i = 256; i < 512; i++) {
        new_pml4_virt[i] = limine_pml4_virt[i];
    }
    g_kernel_ctx.pml_phys = new_pml4_phys;
    g_kernel_ctx.pml_virt = new_pml4_virt;
    vmm_switch_context(&g_kernel_ctx);
}

vmm_context_t* get_current_context(void) {
    return &g_kernel_ctx;
}

void vmm_switch_context(vmm_context_t *new_ctx) {
    asm volatile ("mov %0, %%cr3" :: "r"(ctx->pml4_phys) : "memory");
}

vmmm_map_page ()