#ifndef DLMALLOC_CONFIG_H
#define DLMALLOC_CONFIG_H

#include <stddef.h>
#include <stdint.h>
#include "memory.h"
#include "vmm.h"
#include "pmm.h"

// --- BARE METAL HEADERS CONFIG ---
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#define LACKS_STDLIB_H
#define LACKS_FCNTL_H 1
#define LACKS_TIME_H 1
#define HAVE_SBRK 0   // we provide our own MORECORE, not libc sbrk()

// LACKS_ERRNO_H means dlmalloc.c never pulls in <errno.h>, but posix_memalign()
// still references these two codes directly - supply them ourselves.
#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif

// --- BACKING STORE: classic sbrk-style single contiguous heap ---
// No mmap-backed segments at all - the kernel heap is one growable/shrinkable
// region driven entirely through MORECORE, which is what lets dlmalloc's
// built-in top-chunk trimming actually give pages back on free().
#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define HAS_INLINE_ALLOC 0

#define HAVE_MORECORE 1
#define MORECORE_CONTIGUOUS 1
#define MORECORE_CANNOT_TRIM 0

// Forward declaration of our sbrk-style heap hook
void* sys_sbrk(intptr_t increment);
#define MORECORE(size) sys_sbrk(size)

// Single global heap - no mspaces. This is what makes trimming/shrinking work
// simply: there's exactly one break pointer, exactly one MORECORE.
#define NO_MALLOC_STATS 1
#define MSPACES 0
#define ONLY_MSPACES 0
#define USE_LOCKS 0
#define USE_DEV_RANDOM 0

// Without this, dlmalloc.c emits plain malloc/free/realloc/malloc_trim
// instead of dlmalloc/dlfree/dlrealloc/dlmalloc_trim - which is what heap.c
// actually calls. Required for the extern declarations in heap.c to resolve.
#define USE_DL_PREFIX 1

#define HAVE_MORECORE_ALT 0

#define MALLOC_ALIGNMENT 16
#define MALLOC_PAGE_SIZE 4096

// Trim aggressively-ish: once >=64KB of free space sits at the top of the
// heap, give it back to the VMM/PMM. Granularity matches PAGE_SIZE so every
// MORECORE request lines up on page boundaries.
#define DEFAULT_TRIM_THRESHOLD ((size_t)64 * 1024)
#define DEFAULT_GRANULARITY    ((size_t)4096)

#define ABORT KPANIC(NULL, "dlmalloc assertion failure")
#define MALLOC_FAILURE_ACTION

extern uint64_t timer_get_ticks(void);
#define INITIAL_LOCK_SEED ((size_t)timer_get_ticks())

#include "heap.h"

#endif