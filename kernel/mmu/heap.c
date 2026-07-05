#include "heap.h"

extern mspace create_mspace_with_base(void* base, size_t capacity, int locked);
extern void* mspace_malloc(mspace msp, size_t bytes);
extern void mspace_free(mspace msp, void* mem);
extern void* mspace_realloc(mspace msp, void* mem, size_t newsize);