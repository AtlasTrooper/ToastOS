#include "gdt.h"

static gdtEntry gdt[5];
static tssEntry tss_entry;

static GDT gdtr;
static TSS tss;

void initGDT(void) {
    encode_gdt_seg(0, 0, 0);
    
    encode_gdt_seg(1,
         GDT_PRESENT |
         GDT_DPL_RING0 |
         GDT_DESCRIPTOR | 
         GDT_EXECUTABLE | 
         GDT_READABLE, 
         GDT_LONG_MODE);

    encode_gdt_seg(2,
         GDT_PRESENT |
         GDT_DPL_RING0 |
         GDT_DESCRIPTOR | 
         GDT_WRITEABLE,
        0);

    encode_gdt_seg(3,
         GDT_PRESENT |
         GDT_DPL_RING3 |
         GDT_DESCRIPTOR | 
         GDT_WRITEABLE,
        0);

    encode_gdt_seg(4,
         GDT_PRESENT |
         GDT_DPL_RING3 |
         GDT_DESCRIPTOR | 
         GDT_EXECUTABLE | 
         GDT_READABLE, 
         GDT_LONG_MODE);

    encode_tss_seg((uint64_t)&tss);

    gdtr.lim = sizeof(gdt) + sizeof(tss_entry) - 1;
    gdtr.base = (uint64_t)&gdt;

    gdt_flush(&gdtr);
    tss_flush();
}

void load_tss(uint64_t rsp0) {
    tss.rsp[0] = rsp0;
}

void encode_gdt_seg(int index, uint8_t access, uint8_t gran) {
    gdt[index].base_low = gdt[index].base_mid = gdt[index].base_high = 0;
    gdt[index].lim = 0;
    gdt[index].access = access;
    gdt[index].granularity = gran;
}
void encode_tss_seg(uint64_t base) {
    uint32_t lim = sizeof(TSS)-1;
    tss_entry.base_low = base & 0xFFFF;
    tss_entry.limit_low = lim & 0xFFFF;
    tss_entry.base_high = (base>>16) & 0xFF;
    tss_entry.access = 0x89;
    tss_entry.granularity = ((lim >> 16) & 0x0F);
    tss_entry.base_high = (base >> 24) & 0xFF;
    tss_entry.base_upper = (base >> 32) & 0xFFFFFFFF;
    tss_entry.reserved = 0;
}

extern void gdt_flush(GDT *gdtr);
extern void tss_flush(void);
