#include "qol.h"

void *memset(void *dest, int val, unsigned int iter){
    unsigned char* ptr =dest;
    while(iter--){
        *ptr ++ = (unsigned char) val;
    }
    return dest;
}