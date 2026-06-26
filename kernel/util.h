#pragma once
#include <stddef.h>
#include <stdint.h>

#define PACKED __attribute__((packed))
#define CEIL(data, cap) ((data + (cap-1)) & ~(cap-1))
#define FLOOR(x, a) ((x) & ~((a) - 1))
