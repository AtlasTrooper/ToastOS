#pragma once

/*
Config file for dlmalloc
*/

#define LACKS_UNISTD_H          1
#define LACKS_FCNTL_H           1
#define LACKS_SYS_PARAM_H       1
#define LACKS_SYS_MMAN_H        1
#define LACKS_STRINGS_H         1
#define LACKS_STRING_H          0   /* we have string.h (memset/memcpy)    */
#define LACKS_SYS_TYPES_H       1
#define LACKS_ERRNO_H           1
#define LACKS_STDLIB_H          1
#define LACKS_STDDEF_H          0   /* we have stddef.h                    */
#define LACKS_TIME_H            1

#define HAVE_MMAP               0
#define HAVE_MREMAP             0

#define USE_LOCKS               0
#define USE_SPIN_LOCKS          0

#undef WIN32
#undef _WIN32
#undef _WIN32_WCE

/* ── errno stub ─────────────────────────────────────────────────────────── */
/*
 * dlmalloc sets errno = ENOMEM on OOM.  We don't have a real errno so we
 * give it a throwaway global it can write to harmlessly.
 */
#define ENOMEM  12              /* standard POSIX value                    */
#define EINVAL  22
static int dlmalloc_errno;
#define errno dlmalloc_errno

/* ── fprintf / stderr stub ──────────────────────────────────────────────── */
/*
 * dlmalloc calls fprintf(stderr, ...) for internal corruption messages.
 * We redirect stderr writes to printf and then panic.
 */
#include "../stdlib/stdio.h"
#include "../idt/idt.h"
#define stderr  ((void*)0)      /* satisfies the declaration; never used   */
#define fprintf(stream, fmt, ...) \
    do { printf(fmt, ##__VA_ARGS__); KPANIC(NULL, "dlmalloc internal error"); } while(0)

/* ── abort → panic ──────────────────────────────────────────────────────── */
#define ABORT                   KPANIC(NULL, "dlmalloc: abort")
#define ABORT_ON_ASSERT_FAILURE 1

/* ── mspace ─────────────────────────────────────────────────────────────── */
#define ONLY_MSPACES            1
#define MSPACES                 1

/* ── MORECORE — points to the kernel heap's sbrk ───────────────────────── */
/*
 * dlmalloc 2.8.6 has no per-mspace grow callback.  When an mspace created
 * with create_mspace_with_base() runs out of its initial memory it calls
 * the global MORECORE to get more.  We define MORECORE as a thin wrapper
 * around heap_sbrk() operating on a single pointer (g_morecore_heap) that
 * heap.c sets before creating the mspace.
 *
 * For a single kernel mspace this is fine.  When you add user-process heaps
 * each process will create its own mspace from a pre-sized region and you
 * can switch g_morecore_heap during context switches if needed.
 */
struct heap_t;                          /* forward declaration              */
extern struct heap_t *g_morecore_heap;  /* set by heap.c before use        */
void *heap_sbrk(struct heap_t *heap, long inc);

#define MORECORE(size)  heap_sbrk(g_morecore_heap, (long)(size))
#define MORECORE_CONTIGUOUS 1   /* our sbrk always returns contiguous mem  */
#define MORECORE_CANNOT_TRIM 0

#define MALLOC_ALIGNMENT        16

#define DEFAULT_TRIM_THRESHOLD  (256 * 1024)
#define DEFAULT_MMAP_THRESHOLD  (256 * 1024)   /* irrelevant, mmap=0       */

#define DEBUG                   0   /* flip to 1 when debugging the heap   */