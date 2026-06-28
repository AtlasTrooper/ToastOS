#pragma once
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"

#define VMM_PRESENT   (1ULL <<  0)
#define VMM_WRITE     (1ULL <<  1)
#define VMM_USER      (1ULL <<  2)
#define VMM_PWT       (1ULL <<  3)
#define VMM_PCD       (1ULL <<  4)
#define VMM_ACCESSED  (1ULL <<  5)
#define VMM_DIRTY     (1ULL <<  6)
#define VMM_HUGE      (1ULL <<  7)   /* 2 MB (PD) or 1 GB (PDPT) pages */
#define VMM_GLOBAL    (1ULL <<  8)
#define VMM_NX        (1ULL << 63)   /* No-Execute                      */

#define VMM_FLAGS_KERNEL_RW   (VMM_PRESENT | VMM_WRITE)
#define VMM_FLAGS_KERNEL_RO   (VMM_PRESENT)
#define VMM_FLAGS_USER_RW     (VMM_PRESENT | VMM_WRITE | VMM_USER)
#define VMM_FLAGS_USER_RO     (VMM_PRESENT | VMM_USER)

/* Mask to extract the physical address from an entry
 * (clears flag bits 0-11 and the NX bit 63)               */
#define VMM_ADDR_MASK  0x000FFFFFFFFFF000ULL

/*
 * map_page  – map one 4 KB virtual page to a physical frame.
 *   v_addr   : virtual address  (will be page-aligned internally)
 *   p_addr   : physical address (will be page-aligned internally)
 *   flags    : OR of VMM_* defines above
 *
 * Intermediate tables (PML4 → PDPT → PD → PT) are allocated on demand
 * from the PMM.  The flags are applied to both the leaf PT entry and any
 * newly created intermediate table entries (with WRITE forced on so the
 * kernel can always reach the next level).
 */
void map_page(uint64_t v_addr, uint64_t p_addr, uint64_t flags);

void unmap_page(uint64_t v_addr);

void invalidate_page(uint64_t addr);

uint64_t get_pml4_phys(void);

uint64_t virt_to_phys(uint64_t v_addr);
