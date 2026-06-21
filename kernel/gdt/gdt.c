#include "gdt.h"

static gdtEntry gdt[5];
static tssEntry tss_entry;

static GDT gdtr;
static TSS tss

void initGDT() {
    
}

void encode_gdt_seg(int index, uint8_t access, uint8_t gran) {
    gdt[index].base = gdt[index].base_mid = gdt[index].base_high = 0;
    gdt[index].lim = 0;
    gdt[access] = access;
    gdt[gran] = gran;
}
void encode_tss_seg(uint64_t base) {
    uint32_t lim = sizeof(TSS)-1;
    tss_entry.base_low = base & 0xFFFF;
    tss_entry.lim_low = lim & 0xFFFF;
    tss_entry.base_high = (base>>16) & 0xFF;
    tss_entry.access = 0x89;
    tss_entry.gran = ((lim >> 16) & 0x0F);
    tss_entry.base_high = (base >> 24) & 0xFF;
    tss_entry.base_upper = (base >> 32) & 0xFFFFFFFF;
    tss_entry.reserved = 0;
}

extern void load_gdt(uint32_t addr);
extern void flush();
extern void load_tss();