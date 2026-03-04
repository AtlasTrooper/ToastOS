#include "../qol.h"
#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H
typedef struct PACKED gdtEntry{
    uint16_t limit;
    uint16_t base_low;     
    uint8_t  base_mid;      
    uint8_t  access;      
    uint8_t  flags //+limit granularity;  
    uint8_t  base_high;   
}gdtEntry;

typedef struct PACKED GDT{
    uint16_t lim;
    unsigned int base;
}GDT;

typedef struct PACKED tssEntry{

}tssEntry;

typedef struct PACKED TSS{

}TSS;

void loadGDT(GDT *gdt_ptr);
void flushAndReload(uint32_t);
void initGDT(void);
void encode_gdt_seg(uint32_t index, uint32_t base, uint32_t lim, uint8_t access, uint8_t gran);
void initTSS();
void encode_tss_seg();

#endif