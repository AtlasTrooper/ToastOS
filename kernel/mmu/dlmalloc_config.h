#pragma once

#ifndef DLMALLOC_CONFIG_H
#define DLMALLOC_CONFIG_H

// Freestanding configurations
#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define HAVE_SBRK 0
#define HAS_INLINE_ALLOC 0
#define MALLOC_FAILURE_ACTION
#define NO_MALLOC_STATS 1
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#define LACKS_STDLIB_H

// Core compilation choices
#define MSPACES 1             // CRITICAL: Enables create_mspace_with_base()
#define ONLY_MSPACES 1        // Compiles out global malloc/free to avoid namespaces clashes
#define USE_LOCKS 0           // Set to 0 for now; protect via your own spinlocks in heap_t later

// Forward declare standard functions dlmalloc expects
#include <stddef.h>
#include "memory.h"
#include "vmm.h"
#include "pmm.h"
#include "../drivers/timer.h"
#endif // DLMALLOC_CONFIG_H