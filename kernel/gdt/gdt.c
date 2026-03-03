#include "gdt.h"

//entry
gdtEntry ent;
GDT gdt;
void initGDT(){
    gdt.limit = sizeof(ent)-1;
    gdt.base = (uint32_t)&gdt;
    flushAndReload();
}