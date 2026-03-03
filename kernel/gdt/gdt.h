#include "../qol.h"

typedef struct PACKED gdtEntry{
    uint16_t limit_low;
    uint16_t base_low;     
    uint8_t  base_mid;      
    uint8_t  access;      
    uint8_t  granularity;  
    uint8_t  base_high;   
}gdtEntry;

typedef struct PACKED GDT{
    unsigned short lim;
    unsigned int base;
}GDT;

void loadGDT(GDT *gdt_ptr);
void flushAndReload(void);
void initGDT(void);
void encode_gdt_seg();

