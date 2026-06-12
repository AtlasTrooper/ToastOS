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

void *malloc(int size);
void free(void *ptr);

static Header *morebytes(unsigned numOUnits);


