#pragma once
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"
#include "../serial.h"

#define PTE_PRESENT   (1ULL <<  0)
#define PTE_WRITE     (1ULL <<  1)
#define PTE_USER      (1ULL <<  2)
#define PTE_PS        (1ULL <<  7)
#define PTE_NX        (1ULL << 63)   /* only valid once EFER.NXE is on, use vmm_nx_flag() */

#define VMM_FLAGS_KERNEL_RW   (PTE_PRESENT | PTE_WRITE)
#define VMM_FLAGS_KERNEL_RO   (PTE_PRESENT)
#define VMM_FLAGS_USER_RW     (PTE_PRESENT | PTE_WRITE | PTE_USER)
#define VMM_FLAGS_USER_RO     (PTE_PRESENT | PTE_USER)

#define PTE_FRAME_MASK 0x000FFFFFFFFFF000ULL

/* Everything below this is "user half" (PML4 slots 0-255), everything above is
 * the kernel half that is shared by every address space. */
#define USER_SPACE_END 0x0000800000000000ULL

typedef struct vmm_context_t{
    uint64_t *pml_virt;
    uint64_t pml_phys;
} vmm_context_t;

void init_vmm();

/* The kernel's own address space (what the heap maps into). */
vmm_context_t* vmm_get_kernel_context(void);

void vmm_switch_context(vmm_context_t *new_ctx);

/* Per-process address spaces. The new PML4 shares the kernel half with the
 * kernel context; the user half starts empty. */
vmm_context_t* vmm_create_context(void);

/* Frees every user page, every user page-table, the PML4 and the ctx itself.
 * ASSUMES every user leaf frame is owned by this context. Never pass the
 * kernel context. Safe to call on a context that is currently loaded. */
void vmm_destroy_context(vmm_context_t *ctx);

/* Returns 0 on success, -1 on failure (OOM, or USER flag on a kernel-half address). */
int  vmm_map_page(vmm_context_t *ctx, uint64_t v_addr, uint64_t p_addr, uint64_t flags);
void vmm_unmap_page(vmm_context_t *ctx, uint64_t v_addr);

/* Allocates zeroed frames for [v_addr, v_addr+len), optionally copies init_len bytes
 * of `init` to the start, and maps them with flags|PTE_USER. v_addr must be page aligned. */
int  vmm_user_alloc(vmm_context_t *ctx, uint64_t v_addr, size_t len, uint64_t flags,
                    const void *init, size_t init_len);

/* 1 if the whole range is mapped user-accessible (and writable if need_write). */
int  vmm_user_range_ok(vmm_context_t *ctx, uint64_t v_addr, uint64_t len, int need_write);

/* PTE_NX if the CPU supports it and we enabled it, otherwise 0. OR this into
 * flags for data/stack pages. */
uint64_t vmm_nx_flag(void);

uint64_t vmm_get_phys(vmm_context_t *ctx, uint64_t v_addr);
