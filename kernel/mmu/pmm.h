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

typedef struct {
    uint64_t  hhdm_base;
    uint64_t  kernel_phys_base;
    uint64_t  kernel_virt_base;
    uint64_t  kernel_phys_end;
    uint64_t  bitmap_phys;
    uint64_t  bitmap_bytes;
    uint64_t  alloc_start_frame;
    uint64_t  max_frames;
    uint64_t  free_frames;
    uint8_t *b_map;
} pmm_header_t;

const pmm_header_t *get_pmm_header(void);

uint64_t  get_hhdm(void);
void     *get_virt_addr(uint64_t paddr);
uint64_t  get_phys_addr(void *vaddr);
uint64_t  pmm_alloc(void);
void      pmm_free(uint64_t paddr);
void      init_pmm(void);

struct limine_memmap_response *get_memmap(void);