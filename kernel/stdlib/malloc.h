#include "../memoryMan/memory.h"

//most restrictive possible align type
typedef long align_tight;

typedef union mem_blk_header{
    struct{
        union mem_blk_header * next;
        unsigned size;
    }data;
    align_tight x; //alligning to worst-case boundry
};

typedef union mem_blk_header Header;

static Header base;
static Header *free_lst = NULL;

//kernel space malloc
void * kmalloc(int size);
void *malloc(int size);
void free(void *p);

// static Header *morebytes(unsigned numOUnits);


