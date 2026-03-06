#include "gdt.h"
#include "serial.h"
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
tss_segment tss;
GDT gdt;
void initGDT(){
    gdt.lim = (sizeof(gdtEntry)*6)-1; //Cause we got 6
    gdt.base = (uint32_t)&ent;
    //debug_print("Lim and base prepped");
    //encode our 6 entries
    encode_gdt_seg(0, 0, 0, 0, 0);
    encode_gdt_seg(1, 0, 0xFFFFF, 0x9A, 0xC0);
    encode_gdt_seg(2, 0, 0xFFFFF, 0x92, 0xC0);

    encode_gdt_seg(3, 0, 0xFFFFF, 0xFA, 0xC0);
    encode_gdt_seg(4, 0, 0xFFFFF, 0xF2, 0xC0);

    encode_tss_seg(5, 0x10, 0x00);

    //debug_print("entries encoded");

    load_gdt((uint32_t)&gdt);
    //debug_print("GDT LOADED");
    flush();
    //debug_print("Flushed");
    load_tss();
    
    //debug_print("post asm functions");
    debug_print("GDT IS UP AND RUNNING");
    
}
void encode_gdt_seg(uint32_t index, uint32_t base, uint32_t lim, uint8_t access, uint8_t gran){
    ent[index].base_low = (base & 0xFFFF);
    ent[index].base_mid = (base >> 16)&0xFF;
    ent[index].base_high = (base >> 24) & 0xFF;

    ent[index].lim = (lim & 0xFFFF);//takes limit low
    ent[index].access = access;

    //Takes our limit high and combines it with our granularity flag
    ent[index].flags = ((lim >> 16) & 0x0F) |(gran & 0xF0);

}

void encode_tss_seg(uint32_t index, uint32_t ss0, uint32_t esp0){
    /*Base: TSS addr, 
     Lim: size of the tss,
     0x89 as access,
     0x40 as flags,
     
     SS0 gets 0x10,
     ESP0 gets val of sp during system call,
     IOPB gets sizeof(TSS) unless we want to use it further*/
    encode_gdt_seg(index,(uint32_t)&tss, sizeof(tss), 0x89, 0x40);

    memset(&tss, 0, sizeof(tss));

    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.iomap_base = sizeof(tss);

    tss.cs = 0x0b;
    tss.ss = 0x13;
    tss.ds = 0x13;
    tss.es = 0x13;
    tss.fs = 0x13;
    tss.gs = 0x13;

}
