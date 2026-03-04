#include "gdt.h"
#define SEGMENT_COUNT 6
/*GDT Structure:
1. Null descriptor
2. Kernel mode CS
3. Kernel mode DS
4. User mode DS
5. User mode CS
6. TSS
*/
gdtEntry ent[SEGMENT_COUNT];
GDT gdt;
void initGDT(){
    gdt.limit = (sizeof(gdtEntry)*6)-1; //Cause we got 6
    gdt.base = (uint32_t)&ent;

    //encode our 6 entries
    encode_gdt_seg(0, 0, 0x00000000, 0x00, 0x0);
    encode_gdt_seg(1, 0, 0xFFFFF, 0x9A, 0xC);
    encode_gdt_seg(2, 0, 0xFFFFF, 0x92, 0xC);

    encode_gdt_seg(3, 0, 0xFFFFF, 0xFA, 0xC);
    encode_gdt_seg(4, 0, 0xFFFFF, 0xF2, 0xC);

    encode_gdt_seg(5, 0, 0x00000000, 0x00, 0x0);


    flushAndReload((uint32_t)&gdt);
    //also flush the TSS
}
void encode_gdt_seg(uint32_t index, uint32_t base, uint32_t lim, uint8_t access, uint8_t gran){
    ent[index].base_low = (base & 0xFFFF);
    ent[index].base_mid = (base >> 16)&0xFF;
    ent[index].base_high = (base >> 24) & 0xFF;

    ent[index].lim = (lim & 0xFFFF);
    ent[index].access = access;
    ent[index].flags = ((lim >> 16) & 0x0F) |(gran & 0xF0);

}

