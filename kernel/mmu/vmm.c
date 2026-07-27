#include "vmm.h"

static uint64_t g_hhdm_off = 0;
static vmm_context_t g_kernel_ctx;

void init_vmm() {
    g_hhdm_off = get_hhdm();

    uint64_t initial_cr3;
    asm volatile("mov %%cr3, %0" : "=r"(initial_cr3));
    uint64_t *limine_pml4_virt = (uint64_t*)(initial_cr3 + g_hhdm_off);

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

void vmm_switch_context(vmm_context_t *ctx) {
    asm volatile ("mov %0, %%cr3" :: "r"(ctx->pml_phys) : "memory");
}

static uint64_t* get_or_alloc_table(uint64_t *table, uint64_t idx, uint64_t flags) {
    if (!(table[idx] & PTE_PRESENT)) {
        uint64_t new_table_phys = pmm_alloc();
        if (!new_table_phys) return NULL;
        
        uint64_t *new_table_virt = (uint64_t*)get_virt_addr(new_table_phys);
        memset(new_table_virt, 0, PAGE_SIZE);
        table[idx] = new_table_phys | PTE_PRESENT | PTE_WRITE | flags;
    }
    return (uint64_t*)get_virt_addr(table[idx] & PTE_FRAME_MASK);
}

void vmm_map_page(vmm_context_t *ctx, uint64_t v_addr, uint64_t p_addr, uint64_t flags) {
    uint64_t pml4_idx = (v_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (v_addr >> 30) & 0x1FF;
    uint64_t pd_idx   = (v_addr >> 21) & 0x1FF;
    uint64_t pt_idx   = (v_addr >> 12) & 0x1FF;

    uint64_t *pml4 = ctx->pml_virt;
    uint64_t *pdpt = get_or_alloc_table(pml4, pml4_idx, flags);
    if (!pdpt) return;

    uint64_t *pd = get_or_alloc_table(pdpt, pdpt_idx, flags);
    if (!pd) return;

    uint64_t *pt = get_or_alloc_table(pd, pd_idx, flags);
    if (!pt) return;

    pt[pt_idx] = (p_addr & PTE_FRAME_MASK) | PTE_PRESENT | flags;
    asm volatile ("invlpg (%0)" :: "r"(v_addr) : "memory");
}

uint64_t vmm_get_phys(vmm_context_t *ctx, uint64_t v_addr) {
    uint64_t pml4_idx = (v_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (v_addr >> 30) & 0x1FF;
    uint64_t pd_idx   = (v_addr >> 21) & 0x1FF;
    uint64_t pt_idx   = (v_addr >> 12) & 0x1FF;

    uint64_t *pml4 = ctx->pml_virt;
    if (!(pml4[pml4_idx] & PTE_PRESENT)) return 0;

    uint64_t *pdpt = (uint64_t *)get_virt_addr(pml4[pml4_idx] & PTE_FRAME_MASK);
    if (!(pdpt[pdpt_idx] & PTE_PRESENT)) return 0;

    uint64_t *pd = (uint64_t *)get_virt_addr(pdpt[pdpt_idx] & PTE_FRAME_MASK);
    if (!(pd[pd_idx] & PTE_PRESENT)) return 0;

    uint64_t *pt = (uint64_t *)get_virt_addr(pd[pd_idx] & PTE_FRAME_MASK);
    if (!(pt[pt_idx] & PTE_PRESENT)) return 0;

    return (pt[pt_idx] & PTE_FRAME_MASK) | (v_addr & 0xFFF);
}

void vmm_unmap_page(vmm_context_t *ctx, uint64_t v_addr) {
    uint64_t pml4_idx = (v_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (v_addr >> 30) & 0x1FF;
    uint64_t pd_idx   = (v_addr >> 21) & 0x1FF;
    uint64_t pt_idx   = (v_addr >> 12) & 0x1FF;

    uint64_t *pml4 = ctx->pml_virt;

    if (!(pml4[pml4_idx] & PTE_PRESENT)) return;
    uint64_t *pdpt = (uint64_t *)get_virt_addr(pml4[pml4_idx] & PTE_FRAME_MASK);

    if (!(pdpt[pdpt_idx] & PTE_PRESENT)) return;
    uint64_t *pd = (uint64_t *)get_virt_addr(pdpt[pdpt_idx] & PTE_FRAME_MASK);

    if (!(pd[pd_idx] & PTE_PRESENT)) return;
    uint64_t *pt = (uint64_t *)get_virt_addr(pd[pd_idx] & PTE_FRAME_MASK);

    if (!(pt[pt_idx] & PTE_PRESENT)) return;

    pt[pt_idx] = 0;
    asm volatile ("invlpg (%0)" :: "r"(v_addr) : "memory");
}