#include "memory.h"

//most restrictive possible align type
typedef long align_tight;

typedef union mem_blk_header{
    struct{
        mem_blk_header * next;
        unsigned uint32_t size;
    }data;
    align_tight x; //alligning to worst-case boundry
};

typedef mem_blk_header Header;

static mem_blk_header base;
static mem_blk_header *free_lst = NULL;

static Header * morebytes(unsigned bc);

//kernel space malloc
void * kmalloc(int size);
void free(void *p);