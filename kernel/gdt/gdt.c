#include "gdt.h"
#include "../stdlib/stdio.h"
static struct gdt_table{
    gdtEntry entries[5];
    tssEntry tss;
}PACKED gdt_table;

static GDT_descriptor gdtr;
static TSS_descriptor tss;

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

    gdtr.lim = sizeof(gdt_table) - 1;
    gdtr.base = (uint64_t)&gdt_table;

    printf("gdt_table size: %d\n", (int)sizeof(gdt_table));
    printf("gdtEntry size: %d\n", (int)sizeof(gdtEntry));
    printf("tssEntry size: %d\n", (int)sizeof(tssEntry));

    gdt_flush(&gdtr);
    tss_flush();
}

void load_tss(uint64_t rsp0) {
    tss.rsp[0] = rsp0;
}

void encode_gdt_seg(int index, uint8_t access, uint8_t gran) {
    gdt_table.entries[index].base_low = 0;
    gdt_table.entries[index].base_mid = 0;
    gdt_table.entries[index].base_high = 0;
    gdt_table.entries[index].lim = 0;
    gdt_table.entries[index].access = access;
    gdt_table.entries[index].granularity = gran;
}
void encode_tss_seg(uint64_t base) {
    uint32_t lim = sizeof(TSS_descriptor)-1;
    gdt_table.tss.base_low = base & 0xFFFF;
    gdt_table.tss.limit_low = lim & 0xFFFF;
    gdt_table.tss.base_mid = (base>>16) & 0xFF;
    gdt_table.tss.access = 0x89;
    gdt_table.tss.granularity = ((lim >> 16) & 0x0F);
    gdt_table.tss.base_high = (base >> 24) & 0xFF;
    gdt_table.tss.base_upper = (base >> 32) & 0xFFFFFFFF;
    gdt_table.tss.reserved = 0;
}

extern void gdt_flush(GDT_descriptor *gdtr);
extern void tss_flush(void);
