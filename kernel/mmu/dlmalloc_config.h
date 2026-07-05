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
#define LACKS_TIME_H 1

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



/* ── Feed dlmalloc with your custom timer driver ── */
// Option A: If your driver exposes a tick function
extern uint64_t timer_get_ticks(void);
#define INITIAL_LOCK_SEED  ((size_t)timer_get_ticks())

// Option B: If your driver exposes a global volatile tick count instead
// extern volatile uint64_t system_ticks;
// #define INITIAL_LOCK_SEED  ((size_t)system_ticks)

#ifndef ABORT
#define ABORT KPANIC(NULL, "dlmalloc: internal assertion failure")
#endif
#define ABORT_ON_ASSERT_FAILURE 1


#endif // DLMALLOC_CONFIG_H