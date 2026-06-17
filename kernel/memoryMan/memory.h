#pragma once
#include "../qol.h"
#include "multiboot.h"
#include "../stdlib/stdio.h"
#include "../serial.h"
#define KERNEL_START 0xC0000000
#define CEIL(data, cap) ((data + (cap-1)) & ~(cap-1))

void _kernel_v_start(void);
void _kernel_p_start(void);
void _kernel_v_end(void);
void _kernel_p_end(void);

void *memset(void *dest, int val, unsigned int iter);
