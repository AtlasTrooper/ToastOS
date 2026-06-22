
#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H
#include <stddef.h>
#include <stdint.h>
#include "../mmu/memory.h"
#include "../util.h"

#define SEGMENT_COUNT 5

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_DATA   0x18
#define GDT_USER_CODE   0x20
#define GDT_TSS         0x28

#define GDT_PRESENT        (1 << 7)
#define GDT_DPL_RING0      (0 << 5)
#define GDT_DPL_RING3      (3 << 5)
#define GDT_DESCRIPTOR     (1 << 4)  // 1 = code/data, 0 = system
#define GDT_EXECUTABLE     (1 << 3)
#define GDT_READABLE       (1 << 1)  // for code segments
#define GDT_WRITEABLE       (1 << 1)  // for data segments

#define GDT_LONG_MODE      (1 << 5)  // L bit — 64-bit code segment
#define GDT_GRANULARITY    (1 << 7)  // page granularity

typedef struct PACKED gdtEntry{
    uint16_t lim;
    uint16_t base_low;     
    uint8_t  base_mid;      
    uint8_t  access;      
    uint8_t  granularity; // flags + limit granularity;  
    uint8_t  base_high;   
}gdtEntry;

typedef struct PACKED tssEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
}tssEntry;

typedef struct PACKED GDT{
    uint16_t lim;
    unsigned int base;
}GDT;

typedef struct PACKED TSS{
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb;
}TSS;

void initGDT(void);
void load_tss(uint64_t rsp0);
void encode_gdt_seg(int index, uint8_t access, uint8_t gran);
void encode_tss_seg(uint64_t base);

extern void gdt_flush(GDT *gdtr);
extern void tss_flush(void);

#endif