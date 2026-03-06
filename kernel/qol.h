#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PACKED __attribute__((packed))

void *memset(void *dest, int val, unsigned int iter);