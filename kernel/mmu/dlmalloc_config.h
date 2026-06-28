#pragma once

/* ═══════════════════════════════════════════════════════════════════════════
 * dlmalloc_config.h
 *
 * Strips dlmalloc down to a bare-metal kernel allocator:
 *   - No OS calls (no sbrk, no mmap, no Win32, no pthreads)
 *   - No C stdlib (no errno, no fprintf, no abort)
 *   - mspace API enabled so each heap_t gets its own arena
 *   - Our heap_sbrk() is the sole memory source
 * ═══════════════════════════════════════════════════════════════════════════
 */

/* ── Kill every OS / platform path ─────────────────────────────────────── */
#define LACKS_UNISTD_H          1   /* no unistd.h                         */
#define LACKS_FCNTL_H           1   /* no fcntl.h                          */
#define LACKS_SYS_PARAM_H       1   /* no sys/param.h                      */
#define LACKS_SYS_MMAN_H        1   /* no mmap                             */
#define LACKS_STRINGS_H         1   /* no strings.h                        */
#define LACKS_STRING_H          0   /* we DO have string.h (memset/memcpy) */
#define LACKS_SYS_TYPES_H       1   /* no sys/types.h                      */
#define LACKS_ERRNO_H           1   /* no errno                            */
#define LACKS_STDLIB_H          1   /* no stdlib.h                         */
#define LACKS_STDDEF_H          0   /* we have stddef.h (size_t etc.)      */
#define LACKS_TIME_H            1   /* no time.h                           */

/* ── Disable mmap entirely ──────────────────────────────────────────────── */
#define HAVE_MMAP               0
#define HAVE_MREMAP             0

/* ── Disable threading ──────────────────────────────────────────────────── */
#define USE_LOCKS               0   /* single-threaded kernel for now      */
#define USE_SPIN_LOCKS          0

/* ── Disable Win32 paths ────────────────────────────────────────────────── */
/*
 * dlmalloc uses #ifdef WIN32 / #ifdef _WIN32, not #if WIN32.
 * Defining WIN32 as 0 still satisfies #ifdef so the Windows branch fires.
 * We must #undef all three symbols so every #ifdef WIN32 is skipped.
 */
#undef WIN32
#undef _WIN32
#undef _WIN32_WCE

/* ── Abort / assert: route to our panic ────────────────────────────────── */
#include "../idt/idt.h"             /* for KPANIC                          */
#define ABORT                   KPANIC(NULL, "dlmalloc: internal abort")
#define ABORT_ON_ASSERT_FAILURE 1

/* ── No footers (smaller allocations, fine for kernel) ─────────────────── */
#define FOOTERS                 0

/* ── mspace: one arena per heap_t ──────────────────────────────────────── */
#define ONLY_MSPACES            1   /* expose only mspace_malloc etc.      */
#define MSPACES                 1

/*
 * MORECORE is intentionally NOT defined here.
 * Each mspace is created with create_mspace_with_base() so dlmalloc never
 * calls MORECORE at all — it only uses the memory we hand it up front, and
 * calls our mspace_more callback when it needs to grow.
 * See heap.c for how we wire heap_sbrk into the mspace.
 */

/* ── Alignment ──────────────────────────────────────────────────────────── */
#define MALLOC_ALIGNMENT        16  /* 16-byte aligned, good for SSE/x86-64 */

/* ── Trim / consolidation thresholds ───────────────────────────────────── */
#define DEFAULT_TRIM_THRESHOLD  (256 * 1024)   /* 256 KB                   */
#define DEFAULT_MMAP_THRESHOLD  (256 * 1024)   /* irrelevant (mmap=0)      */

/* ── Diagnostics ────────────────────────────────────────────────────────── */
#define DEBUG                   0   /* flip to 1 when debugging the heap   */