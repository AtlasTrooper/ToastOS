#include "vmm.h"
#include "heap.h"

static uint64_t g_hhdm_off = 0;
static vmm_context_t g_kernel_ctx;
static int g_nx_enabled = 0;

#define MSR_EFER  0xC0000080
#define EFER_NXE  (1ULL << 11)

static inline uint64_t rdmsr64(uint32_t msr) {
    uint32_t lo, hi;
    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr64(uint32_t msr, uint64_t v) {
    asm volatile("wrmsr" :: "c"(msr), "a"((uint32_t)v), "d"((uint32_t)(v >> 32)));
}

static void vmm_enable_nx(void) {
    uint32_t eax, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000000), "c"(0));
    if (eax < 0x80000001) return;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000001), "c"(0));
    if (!(edx & (1u << 20))) return;
    wrmsr64(MSR_EFER, rdmsr64(MSR_EFER) | EFER_NXE);
    g_nx_enabled = 1;
}

uint64_t vmm_nx_flag(void) {
    return g_nx_enabled ? PTE_NX : 0;
}

void init_vmm() {
    g_hhdm_off = get_hhdm();
    vmm_enable_nx();

    uint64_t initial_cr3;
    asm volatile("mov %%cr3, %0" : "=r"(initial_cr3));
    initial_cr3 &= PTE_FRAME_MASK;
    uint64_t *limine_pml4_virt = (uint64_t*)(initial_cr3 + g_hhdm_off);

    uint64_t new_pml4_phys = pmm_alloc();
    uint64_t *new_pml4_virt = (uint64_t*)get_virt_addr(new_pml4_phys);

    memset(new_pml4_virt, 0, PAGE_SIZE);

    for (int i = 256; i < 512; i++) {
        new_pml4_virt[i] = limine_pml4_virt[i];
    }

    /*
     * Pre-populate every empty kernel-half PML4 slot with a zeroed PDPT.
     * Per-process PML4s copy these slots once at creation time; if a slot were
     * still empty then, a later kernel mapping (heap growth, etc.) would only
     * show up in the kernel context and every existing process would miss it.
     * With the PDPTs pre-allocated, all contexts share the same lower levels.
     */
    for (int i = 256; i < 512; i++) {
        if (new_pml4_virt[i] & PTE_PRESENT) continue;
        uint64_t pdpt_phys = pmm_alloc();
        if (!pdpt_phys) KPANIC(NULL, "vmm: out of memory pre-populating kernel PML4");
        memset(get_virt_addr(pdpt_phys), 0, PAGE_SIZE);
        new_pml4_virt[i] = pdpt_phys | PTE_PRESENT | PTE_WRITE;
    }

    g_kernel_ctx.pml_phys = new_pml4_phys;
    g_kernel_ctx.pml_virt = new_pml4_virt;
    vmm_switch_context(&g_kernel_ctx);
}

vmm_context_t* vmm_get_kernel_context(void) {
    return &g_kernel_ctx;
}

void vmm_switch_context(vmm_context_t *ctx) {
    asm volatile ("mov %0, %%cr3" :: "r"(ctx->pml_phys) : "memory");
}

vmm_context_t* vmm_create_context(void) {
    vmm_context_t *ctx = (vmm_context_t*)kmalloc(sizeof(vmm_context_t));
    if (!ctx) return NULL;

    uint64_t phys = pmm_alloc();
    if (!phys) { kfree(ctx); return NULL; }

    uint64_t *virt = (uint64_t*)get_virt_addr(phys);
    memset(virt, 0, PAGE_SIZE);

    for (int i = 256; i < 512; i++) {
        virt[i] = g_kernel_ctx.pml_virt[i];
    }

    ctx->pml_phys = phys;
    ctx->pml_virt = virt;
    return ctx;
}

/* level 3 = PDPT, 2 = PD, 1 = PT (whose present entries are leaf frames) */
static void free_table(uint64_t table_phys, int level) {
    uint64_t *t = (uint64_t*)get_virt_addr(table_phys);
    for (int i = 0; i < 512; i++) {
        uint64_t e = t[i];
        if (!(e & PTE_PRESENT)) continue;
        if (level == 1) {
            pmm_free(e & PTE_FRAME_MASK);
        } else if (!(e & PTE_PS)) {
            free_table(e & PTE_FRAME_MASK, level - 1);
        }
    }
    pmm_free(table_phys);
}

void vmm_destroy_context(vmm_context_t *ctx) {
    if (!ctx || ctx == &g_kernel_ctx) return;

    uint64_t cur_cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cur_cr3));
    if ((cur_cr3 & PTE_FRAME_MASK) == ctx->pml_phys) {
        vmm_switch_context(&g_kernel_ctx);
    }

    for (int i = 0; i < 256; i++) {
        uint64_t e = ctx->pml_virt[i];
        if (e & PTE_PRESENT) free_table(e & PTE_FRAME_MASK, 3);
    }

    pmm_free(ctx->pml_phys);
    kfree(ctx);
}

//if we go shopping in hhdm the world will explode, that's why we go to h&m instead
static uint64_t* get_or_alloc_table(uint64_t *table, uint64_t idx, uint64_t flags,
                                     uint64_t v_addr, const char *level) {

    if (table[idx] & PTE_PRESENT) {
        if (table[idx] & PTE_PS) {
            KPANIC(NULL,
                "vmm: huge page collision walking to %s for v_addr=0x%016llx\n"
                "     table(virt)=0x%016llx idx=%llu raw_entry=0x%016llx\n"
                "     entry_frame=0x%016llx requested_flags=0x%llx",
                level, v_addr,
                (uint64_t)table, idx, table[idx],
                table[idx] & PTE_FRAME_MASK, flags);
        }
        /* The CPU needs U/S set at EVERY level for ring3 to reach a page, so an
         * intermediate table that was first created for a kernel mapping has to
         * be upgraded when a user mapping lands under it. */
        if ((flags & PTE_USER) && !(table[idx] & PTE_USER)) {
            table[idx] |= PTE_USER;
        }
        return (uint64_t*)get_virt_addr(table[idx] & PTE_FRAME_MASK);
    }

    uint64_t new_table_phys = pmm_alloc();
    if (!new_table_phys) return NULL;

    uint64_t *new_table_virt = (uint64_t*)get_virt_addr(new_table_phys);
    memset(new_table_virt, 0, PAGE_SIZE);

    table[idx] = new_table_phys | PTE_PRESENT | PTE_WRITE | (flags & PTE_USER);

    return new_table_virt;
}

int vmm_map_page(vmm_context_t *ctx, uint64_t v_addr, uint64_t p_addr, uint64_t flags) {
    /* user mappings may never live in the shared kernel half */
    if ((flags & PTE_USER) && v_addr >= USER_SPACE_END) return -1;

    uint64_t pml4_idx = (v_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (v_addr >> 30) & 0x1FF;
    uint64_t pd_idx   = (v_addr >> 21) & 0x1FF;
    uint64_t pt_idx   = (v_addr >> 12) & 0x1FF;

    uint64_t *pml4 = ctx->pml_virt;

    uint64_t *pdpt = get_or_alloc_table(pml4, pml4_idx, flags, v_addr, "PDPT");
    if (!pdpt) return -1;

    uint64_t *pd = get_or_alloc_table(pdpt, pdpt_idx, flags, v_addr, "PD");
    if (!pd) return -1;

    uint64_t *pt = get_or_alloc_table(pd, pd_idx, flags, v_addr, "PT");
    if (!pt) return -1;

    /* flags are used as given now (this used to force PTE_WRITE, which made
     * read-only mappings impossible) */
    pt[pt_idx] = (p_addr & PTE_FRAME_MASK) | PTE_PRESENT | flags;
    asm volatile ("invlpg (%0)" :: "r"(v_addr) : "memory");
    return 0;
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
    if (pdpt[pdpt_idx] & PTE_PS) {
        // 1GB huge page at the PDPT level
        return (pdpt[pdpt_idx] & PTE_FRAME_MASK) | (v_addr & 0x3FFFFFFFULL);
    }

    uint64_t *pd = (uint64_t *)get_virt_addr(pdpt[pdpt_idx] & PTE_FRAME_MASK);
    if (!(pd[pd_idx] & PTE_PRESENT)) return 0;
    if (pd[pd_idx] & PTE_PS) {
        // 2MB huge page at the PD level
        return (pd[pd_idx] & PTE_FRAME_MASK) | (v_addr & 0x1FFFFFULL);
    }

    uint64_t *pt = (uint64_t *)get_virt_addr(pd[pd_idx] & PTE_FRAME_MASK);
    if (!(pt[pt_idx] & PTE_PRESENT)) return 0;

    return (pt[pt_idx] & PTE_FRAME_MASK) | (v_addr & 0xFFFULL);
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
    if (pdpt[pdpt_idx] & PTE_PS) {
        KPANIC(NULL,
            "vmm: attempted to unmap 4K page v_addr=0x%016llx inside a 1GB huge "
            "mapping (PDPT entry=0x%016llx)", v_addr, pdpt[pdpt_idx]);
    }
    uint64_t *pd = (uint64_t *)get_virt_addr(pdpt[pdpt_idx] & PTE_FRAME_MASK);

    if (!(pd[pd_idx] & PTE_PRESENT)) return;
    if (pd[pd_idx] & PTE_PS) {
        KPANIC(NULL,
            "vmm: attempted to unmap 4K page v_addr=0x%016llx inside a 2MB huge "
            "mapping (PD entry=0x%016llx)", v_addr, pd[pd_idx]);
    }
    uint64_t *pt = (uint64_t *)get_virt_addr(pd[pd_idx] & PTE_FRAME_MASK);

    if (!(pt[pt_idx] & PTE_PRESENT)) return;

    pt[pt_idx] = 0;
    asm volatile ("invlpg (%0)" :: "r"(v_addr) : "memory");
}

int vmm_user_alloc(vmm_context_t *ctx, uint64_t v_addr, size_t len, uint64_t flags,
                   const void *init, size_t init_len) {
    if (v_addr & (PAGE_SIZE - 1)) return -1;
    if (len == 0) return 0;
    if (v_addr >= USER_SPACE_END || len > USER_SPACE_END - v_addr) return -1;
    if (init_len > len) return -1;

    size_t pages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < pages; i++) {
        uint64_t phys = pmm_alloc();
        if (!phys) return -1;

        /* frames are zeroed so a process never sees stale kernel/other-process data */
        uint8_t *dst = (uint8_t*)get_virt_addr(phys);
        memset(dst, 0, PAGE_SIZE);

        size_t off = i * PAGE_SIZE;
        if (init && off < init_len) {
            size_t n = init_len - off;
            if (n > PAGE_SIZE) n = PAGE_SIZE;
            memcpy(dst, (const uint8_t*)init + off, n);
        }

        if (vmm_map_page(ctx, v_addr + off, phys, flags | PTE_USER) != 0) {
            pmm_free(phys);
            return -1;
        }
    }
    return 0;
}

/* 1 if the page containing v_addr is user-accessible at every level.
 * *writable is set if every level also allows writes. */
static int walk_user(vmm_context_t *ctx, uint64_t v_addr, int *writable) {
    uint64_t *t = ctx->pml_virt;
    int w = 1;

    uint64_t idx[4] = {
        (v_addr >> 39) & 0x1FF, (v_addr >> 30) & 0x1FF,
        (v_addr >> 21) & 0x1FF, (v_addr >> 12) & 0x1FF
    };

    for (int lvl = 0; lvl < 4; lvl++) {
        uint64_t e = t[idx[lvl]];
        if (!(e & PTE_PRESENT) || !(e & PTE_USER)) return 0;
        if (!(e & PTE_WRITE)) w = 0;
        if (lvl == 3) break;
        if (e & PTE_PS) return 0;       /* no huge pages in the user half */
        t = (uint64_t*)get_virt_addr(e & PTE_FRAME_MASK);
    }

    *writable = w;
    return 1;
}

int vmm_user_range_ok(vmm_context_t *ctx, uint64_t v_addr, uint64_t len, int need_write) {
    if (!ctx) return 0;
    if (len == 0) return 1;
    if (v_addr >= USER_SPACE_END || len > USER_SPACE_END - v_addr) return 0;

    uint64_t start = v_addr & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t end   = (v_addr + len + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t page = start; page < end; page += PAGE_SIZE) {
        int writable = 0;
        if (!walk_user(ctx, page, &writable)) return 0;
        if (need_write && !writable) return 0;
    }
    return 1;
}
