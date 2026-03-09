#include "../qol.h"

typedef struct PACKED IDT{
    uint32_t base;
    uint32_t lim; //sizeof IDT -1
}IDT;

typedef struct PACKED idt_entry{
    uint16_t base_lo;
    uint16_t selector;
    uint8_t zero; //reserved
    uint8_t flags; //includes type,dpl,p and the zeroed out bit 44
    uint16_t base_hi;
}idt_entry;

/* Example flag values
32-bit Interrupt Gate: 0x8E (p=1, dpl=0b00, type=0b1110 => type_attributes=0b1000_1110=0x8E)
32-bit Trap Gate: 0x8F (p=1, dpl=0b00, type=0b1111 => type_attributes=1000_1111b=0x8F)
Task Gate: 0x85 (p=1, dpl=0b00, type=0b0101 => type_attributes=0b1000_0101=0x85)
*/

extern void loadIDT(uint32_t);