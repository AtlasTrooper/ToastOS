#include "vmm.h"

/* ── Helpers ────────────────────────────────────────────────────────────── */

/*
 * pte_to_virt – given a page-table entry that holds a physical address in
 * its upper bits, return the HHDM virtual pointer to that physical page.
 */
static inline uint64_t *pte_to_virt(uint64_t entry) {
    return (uint64_t *)get_virt_addr(entry & VMM_ADDR_MASK);
}

/*
 * alloc_table – allocate and zero a fresh 4 KB page-table page.
 * Returns its physical address, or 0 on OOM.
 */
static uint64_t alloc_table(void) {
    uint64_t phys = alloc_frame();
    if (phys == 0) return 0;

    uint64_t *virt = (uint64_t *)get_virt_addr(phys);
    for (int i = 0; i < 512; i++) virt[i] = 0;

    return phys;
}

/*
 * A 64-bit canonical virtual address is split as:
 *   [63:48]  sign extension (ignored by hardware)
 *   [47:39]  PML4 index   (9 bits)
 *   [38:30]  PDPT index   (9 bits)
 *   [29:21]  PD   index   (9 bits)
 *   [20:12]  PT   index   (9 bits)
 *   [11: 0]  page offset  (12 bits)
 */
#define PML4_IDX(va)  (((va) >> 39) & 0x1FFULL)
#define PDPT_IDX(va)  (((va) >> 30) & 0x1FFULL)
#define PD_IDX(va)    (((va) >> 21) & 0x1FFULL)
#define PT_IDX(va)    (((va) >> 12) & 0x1FFULL)

void invalidate_page(uint64_t addr) {
    asm volatile ("invlpg (%0)" :: "r"(addr) : "memory");
}

uint64_t get_pml4_phys(void) {
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3 & VMM_ADDR_MASK;
}

/*
 * map_page
 *
 * Walk PML4 → PDPT → PD → PT, allocating any missing intermediate tables
 * along the way, then write the final PT entry.
 *
 * Intermediate entries always get (flags | VMM_WRITE) so every kernel
 * path can traverse them, regardless of the leaf permissions requested.
 */
void map_page(uint64_t v_addr, uint64_t p_addr, uint64_t flags) {
    v_addr &= VMM_ADDR_MASK;
    p_addr &= VMM_ADDR_MASK;

    /* ── Level 4 : PML4 ── */
    uint64_t *pml4 = (uint64_t *)get_virt_addr(get_pml4_phys());
    uint64_t  pml4e = pml4[PML4_IDX(v_addr)];

    if (!(pml4e & VMM_PRESENT)) {
        uint64_t phys = alloc_table();
        if (phys == 0) { /* OOM – leave unmapped */ return; }
        pml4[PML4_IDX(v_addr)] = phys | flags | VMM_WRITE | VMM_PRESENT;
        pml4e = pml4[PML4_IDX(v_addr)];
    }

    /* ── Level 3 : PDPT ── */
    uint64_t *pdpt = pte_to_virt(pml4e);
    uint64_t  pdpte = pdpt[PDPT_IDX(v_addr)];

    if (!(pdpte & VMM_PRESENT)) {
        uint64_t phys = alloc_table();
        if (phys == 0) return;
        pdpt[PDPT_IDX(v_addr)] = phys | flags | VMM_WRITE | VMM_PRESENT;
        pdpte = pdpt[PDPT_IDX(v_addr)];
    }

    /* ── Level 2 : PD ── */
    uint64_t *pd  = pte_to_virt(pdpte);
    uint64_t  pde = pd[PD_IDX(v_addr)];

    if (!(pde & VMM_PRESENT)) {
        uint64_t phys = alloc_table();
        if (phys == 0) return;
        pd[PD_IDX(v_addr)] = phys | flags | VMM_WRITE | VMM_PRESENT;
        pde = pd[PD_IDX(v_addr)];
    }

    /* ── Level 1 : PT (leaf) ── */
    uint64_t *pt = pte_to_virt(pde);
    pt[PT_IDX(v_addr)] = p_addr | flags;

    invalidate_page(v_addr);
}

/*
 * unmap_page
 *
 * Walk to the PT leaf entry, free the backing physical frame, and clear
 * the entry.  Does NOT free empty intermediate tables (they stay around
 * for future mappings – add that only if you need a full-featured MM).
 */
void unmap_page(uint64_t v_addr) {
    v_addr &= VMM_ADDR_MASK;

    /* Level 4 */
    uint64_t *pml4 = (uint64_t *)get_virt_addr(get_pml4_phys());
    uint64_t  pml4e = pml4[PML4_IDX(v_addr)];
    if (!(pml4e & VMM_PRESENT)) return;

    /* Level 3 */
    uint64_t *pdpt = pte_to_virt(pml4e);
    uint64_t  pdpte = pdpt[PDPT_IDX(v_addr)];
    if (!(pdpte & VMM_PRESENT)) return;

    /* Level 2 */
    uint64_t *pd  = pte_to_virt(pdpte);
    uint64_t  pde = pd[PD_IDX(v_addr)];
    if (!(pde & VMM_PRESENT)) return;

    /* Level 1 – leaf */
    uint64_t *pt  = pte_to_virt(pde);
    uint64_t  pte = pt[PT_IDX(v_addr)];
    if (!(pte & VMM_PRESENT)) return;

    free_frame(pte & VMM_ADDR_MASK);
    pt[PT_IDX(v_addr)] = 0;

    invalidate_page(v_addr);
}

uint64_t virt_to_phys(uint64_t v_addr) {
    uint64_t offset = v_addr & 0xFFF;
    v_addr &= VMM_ADDR_MASK;

    uint64_t *pml4 = (uint64_t *)get_virt_addr(get_pml4_phys());
    uint64_t  pml4e = pml4[PML4_IDX(v_addr)];
    if (!(pml4e & VMM_PRESENT)) return 0;

    uint64_t *pdpt = pte_to_virt(pml4e);
    uint64_t  pdpte = pdpt[PDPT_IDX(v_addr)];
    if (!(pdpte & VMM_PRESENT)) return 0;

    uint64_t *pd  = pte_to_virt(pdpte);
    uint64_t  pde = pd[PD_IDX(v_addr)];
    if (!(pde & VMM_PRESENT)) return 0;

    uint64_t *pt  = pte_to_virt(pde);
    uint64_t  pte = pt[PT_IDX(v_addr)];
    if (!(pte & VMM_PRESENT)) return 0;

    return (pte & VMM_ADDR_MASK) | offset;
}
