#include "malloc.h"
#define MIN_ALLOC 1024

/*
This is my modified implementation of the "sample storage allocator"
from 'The C programming language second edition' by Kernighan and Ritchie
which utilizes my os' page frame allocation system as an alternative 
to the sbrk routine.

It may be modified in the future.

*/

//user mode dynamic storage allocator
void * malloc(int numObytes){
    Header *p, *prevp;
    Header * morebytes(unsigned bc);
    unsigned numOunits;

    numOunits = (numObytes + sizeof(Header)-1)/sizeof(Header)+1;
    
    //No free list, must create empty one pointing to itself
    if ((prevp = free_lst) == NULL) {
        base.data.next = free_lst = prevp = &base;
        base.data.size = 0;
    }
    for (p = prevp->data.next; ; prevp = p, p = p->data.next) {
        //Found a match
        if (p->data.size >= numOunits) {
            //Found an exact match
            if (p->data.size == numOunits) {
                prevp->data.next = p->data.next;
            } else { /* Allocates the needed fraction of the mem block*/
                p->data.size -= numOunits;
                p += p->data.size;
                p-> data.size = numOunits;
            }
            free_lst = prevp;
            return (void *)(p+1);
        }
        if (p == free_lst) {
            if ((p = morebytes(numOunits)) == NULL) {
                return NULL;
            }
        }
    }

}

// frees up memory and returns to free list

/*
TODO:

free and unmap page frames when possible!
*/
void free(void *ptr) {
    Header *block, *scan;
    block = (Header *)ptr - 1;

    for (scan = free_lst; !(block > scan && block < scan->data.next); scan = scan->data.next) {
        if (scan >= scan->data.next && (block > scan || block < scan->data.next)) {
            break;
        }
    }

    if (block + block->data.size == scan->data.next) {
        block->data.size += scan->data.next->data.next->data.size;
        block->data.next = scan->data.next->data.next;
    } else {
        block->data.next = scan->data.next;
    }

    if (scan + scan->data.size == block) {
        scan->data.size += block->data.size;
        scan->data.next = block->data.next;
    } else {
        scan->data.next = block;
    }

    free_lst = scan;
}


//Asks the OS for more memory, leverages the PFA
static Header *morebytes(unsigned numOunits){
    heap_t* k_heap = k_heap_status();
    char *cp;
    Header *up;

    if(numOunits < MIN_ALLOC) {
        numOunits = MIN_ALLOC;
    }
    cp = heapafus(numOunits * sizeof(Header), k_heap);
    if (cp == (char*)-1) {
        return NULL;
    }
    up = (Header*) cp;
    up -> data.size = numOunits;
    free((void*)(up+1));
    return free_lst;
}