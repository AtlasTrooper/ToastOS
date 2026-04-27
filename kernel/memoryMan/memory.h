#pragma once
#include "../qol.h"
#include "multiboot.h"
#include "../stdlib/stdio.h"

#define PAGE_SIZE 4096
#define KERNEL_START 0xC0000000
#define CEIL(data, cap) ((data + (cap-1)) & ~(cap-1))
#define BITMAP_SET(bit) (b_map[bit/32] |= (1 << (bit%32)))
#define BITMAP_CLEAR(bit) (b_map[bit/32] &= ~(1 << (bit %32)))

void _kernel_v_start(void);
void _kernel_p_start(void);
void _kernel_v_end(void);
void _kernel_p_end(void);

extern uint32_t init_page_dir[1024]; //this is our bitmap
extern uint32_t *b_map;
extern uint32_t max_frames;

void initMem(multiboot_info* boot_data);
void *memset(void *dest, int val, unsigned int iter);