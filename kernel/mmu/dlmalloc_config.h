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

// Fallback tokens dlmalloc needs for type checking
#ifndef PROT_READ
#define PROT_READ 1
#endif
#ifndef PROT_WRITE
#define PROT_WRITE 2
#endif
#ifndef MAP_PRIVATE
#define MAP_PRIVATE 2
#endif
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif
#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

// --- MMAP CONFIGURATION ---
#define HAVE_MMAP 1
#define HAVE_MREMAP 0
#define HAVE_SBRK 0
#define HAS_INLINE_ALLOC 0

// Forward declarations of your heap allocation hooks
void* sys_mmap_alloc(size_t size);
int sys_munmap_free(void* addr, size_t size);

#define MMAP(size)                  sys_mmap_alloc(size)
#define MUNMAP(addr, size)          sys_munmap_free(addr, size)
#define DIRECT_MMAP(size)           sys_mmap_alloc(size)

// Also redefine the defaults to completely shield against any internal fallback paths
#define MMAP_DEFAULT(size)          sys_mmap_alloc(size)
#define MUNMAP_DEFAULT(addr, size)  sys_munmap_free(addr, size)

#define NO_MALLOC_STATS 1
#define MSPACES 1             
#define ONLY_MSPACES 1        
#define USE_LOCKS 0           
#define USE_DEV_RANDOM 0

#define HAVE_MORECORE 0
#define MORECORE_CANNOT_INCR 0

#define MALLOC_ALIGNMENT 16
#define MALLOC_PAGE_SIZE 4096

#define DEFAULT_TRIM_THRESHOLD ((size_t)128 * 1024) // Trim when 128KB of continuous top memory is free
#define DEFAULT_GRANULARITY    ((size_t)4096)       // Trim in page-sized increments

#define ABORT KPANIC(NULL, "dlmalloc assertion failure")
#define MALLOC_FAILURE_ACTION

extern uint64_t timer_get_ticks(void);
#define INITIAL_LOCK_SEED ((size_t)timer_get_ticks())

#include "heap.h"

#endif