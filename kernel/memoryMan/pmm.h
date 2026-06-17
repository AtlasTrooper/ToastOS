#pragma once
#include "memory.h"
#include "heap.h"
#include "vmm.h"
#define PAGE_SIZE 4096
#define ALIGN 4096
#define BITMAP_SET(bit) (b_map[bit/32] |= (1 << (bit%32)))
#define BITMAP_CLEAR(bit) (b_map[bit/32] &= ~(1 << (bit %32)))
#define BITMAP_TEST(bit) (b_map[bit/32] & (1 << (bit %32)))

static uintptr_t v_addr_s = (uintptr_t) &_kernel_v_start;
static uintptr_t v_addr_e = (uintptr_t) &_kernel_v_end;
static uintptr_t p_addr_s = (uintptr_t) &_kernel_p_start;
static uintptr_t p_addr_e = (uintptr_t) &_kernel_p_end;

static uint32_t* pd_v_addr = (uint32_t*)(0xFFFFF000);

static uintptr_t p_alloc_s;

static uint32_t *b_map;
static uint32_t max_frames;

extern uintptr_t p_alloc_s;

//Bitmap functions
uint32_t alloc_frame();
void free_frame(uint32_t p_addr);

//PMM
void initPmm(multiboot_info* boot_data);

