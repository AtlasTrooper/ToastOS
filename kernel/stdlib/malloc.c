#include "malloc.h"
#define MIN_ALLOC 1024

//user mode dynamic storage allocator
void * malloc(int numObytes){
    Header *p, *prevp;
    Header * morebytes(unsigned bc);
    unsigned numOunits;

    numOunits = (numObytes + sizeof(Header)-1)/sizeof(Header)+1;
    

}

//kernel mode dynamic storage allocator
void *kmalloc(int numObytes){

}

//frees up memory and returns to free list
void free(void *p){

}


//Asks the OS for more memory, leverages the PFA
// static Header *morebytes(unsigned numOUnits){
//     char *cp, 
// }