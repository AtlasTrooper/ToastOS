
#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H
#include "qol.h"
typedef struct PACKED gdtEntry{
    uint16_t lim;
    uint16_t base_low;     
    uint8_t  base_mid;      
    uint8_t  access;      
    uint8_t  flags; //+limit granularity;  
    uint8_t  base_high;   
}gdtEntry;

typedef struct PACKED GDT{
    uint16_t lim;
    unsigned int base;
}GDT;

typedef struct PACKED tss_segment{
    //32 bit aligned hence why our segment selectors are 32 bit here
    uint32_t prev_tss;//we won't be using this but it's here for legacy reasons

    uint32_t esp0;
    uint32_t ss0;

    uint32_t esp1;
    uint32_t ss1;

    uint32_t esp2;
    uint32_t ss2;

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;

    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;

    uint32_t ldt;

    uint16_t trap;
    uint16_t iomap_base;
}tss_segment;



void initGDT();
void encode_gdt_seg(uint32_t index, uint32_t base, uint32_t lim, uint8_t access, uint8_t gran);
void encode_tss_seg(uint32_t index, uint32_t ss0, uint32_t esp0);

extern void load_gdt(uint32_t addr);
extern void flush();
extern void load_tss();
#endif