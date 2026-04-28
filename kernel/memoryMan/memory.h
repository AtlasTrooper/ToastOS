#pragma once
#include "../qol.h"
#include "multiboot.h"
#include "../stdlib/stdio.h"
#include "../serial.h"
#define PAGE_SIZE 4096
#define KERNEL_START 0xC0000000
#define CEIL(data, cap) ((data + (cap-1)) & ~(cap-1))
#define BITMAP_SET(bit) (b_map[bit/32] |= (1 << (bit%32)))
#define BITMAP_CLEAR(bit) (b_map[bit/32] &= ~(1 << (bit %32)))
#define BITMAP_TEST(bit) (b_map[bit/32] & (1 << (bit %32)))

void _kernel_v_start(void);
void _kernel_p_start(void);
void _kernel_v_end(void);
void _kernel_p_end(void);

extern uint32_t page_directory[1024]; //waht no this is not our bitmap, this is our page dirs, comment was one line too early
static uint32_t *b_map;
static uint32_t max_frames;

void *memset(void *dest, int val, unsigned int iter);
uint32_t alloc_frame();
void free_frame(uint32_t p_addr);

void initMem(multiboot_info* boot_data);
void map_page(uint32_t v_addr, uint32_t p_addr, uint32_t pdt_flags);
void invalidate_page(uint32_t addr);
void reload_CR3(uint32_t p_pd_addr);