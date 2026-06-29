#pragma once
#include "../limine.h"
#include <stddef.h>
#include <stdint.h>
#include "../stdlib/string.h"
#include "../stdlib/stdio.h"
#include "memory.h"
#include "../util.h"
#include "../idt/idt.h"
#include "../shell/tsh.h"

#define PAGE_SIZE       0x1000          /* 4096 bytes                    */
#define PAGE_SHIFT      12              /* log2(PAGE_SIZE), useful later */
#define PAGE_MASK       (~(PAGE_SIZE - 1))

#define BITMAP_SET(bit)   (pmm.b_map[(bit) >> 6] |=  (1ULL << ((bit) & 63)))
#define BITMAP_CLEAR(bit) (pmm.b_map[(bit) >> 6] &= ~(1ULL << ((bit) & 63)))
#define BITMAP_TEST(bit)  (pmm.b_map[(bit) >> 6] &   (1ULL << ((bit) & 63)))

typedef struct {
    uint64_t  hhdm_base;          /* HHDM offset from Limine              */
    uint64_t  kernel_phys_base;   /* where the kernel was loaded           */
    uint64_t  kernel_virt_base;   /* kernel virtual base                   */
    uint64_t  kernel_phys_end;    /* first byte past kernel+modules        */
    uint64_t  bitmap_phys;        /* physical address of the bitmap        */
    uint64_t  bitmap_bytes;       /* size of the bitmap in bytes           */
    uint64_t  alloc_start_frame;  /* first frame alloc_frame will return   */
    uint64_t  max_frames;         /* total frames tracked by the bitmap    */
    uint64_t  free_frames;        /* live count of free frames             */
    uint64_t *b_map;              /* the bitmap itself                     */
} pmm_header_t;

const pmm_header_t *get_pmm_header(void);

uint64_t  get_hhdm(void);
void     *get_virt_addr(uint64_t paddr);
uint64_t  get_phys_addr(void *vaddr);
uint64_t  alloc_frame(void);
void      free_frame(uint64_t paddr);
void      init_pmm(void);

struct limine_memmap_response *get_memmap(void);