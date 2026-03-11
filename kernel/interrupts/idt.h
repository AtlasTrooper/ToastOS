#include "../qol.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define EOI 0x20 //end of interrupt
typedef struct PACKED IDT{
    uint32_t base;
    uint16_t lim; //sizeof IDT -1
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

typedef struct PACKED system_state{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t interr_num, error_code;
    uint32_t eip, cs, eflags, useresp, ss;
}system_state;

extern void loadIDT(uint32_t);

unsigned char *excep_trace[] = {
    "Divide Error (#DE)",
    "Debug Exception (#DB)",
    "NMI Interrupt (NMI)",
    "Breakpoint (#BP)",
    "Overflow (#OF)",
    "BOUND Range Exceeded (#BR)",
    "Invalid Opcode (#UD)",
    "Device Not Available (#NM)",
    "Double Fault (#DF)",
    "Coprocessor Segment Overrun",
    "Invalid TSS (#TS)",
    "Segment Not Present (#NP)",
    "Stack-Segment Fault (#SS)",
    "General Protection (#GP)",
    "Page Fault (#PF)",
    "Intel Reserved",
    "x87 FPU Floating-Point Error (#MF)",
    "Alignment Check (#AC)",
    "Machine Check (#MC)",
    "SIMD Floating-Point Exception (#XM)",
    "Virtualization Exception (#VE)",
    "Control Protection Exception (#CP)",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void * irq_map[16]{
    0,0,0,0,0,0,0,0
    0,0,0,0,0,0,0,0
}

void initIDT();
void encode_interrupt_gate(uint32_t index, uint32_t base, uint16_t sel, uint8_t flags);
void isr_handler(system_state *system);
void irq_handler(system_state *sys);
void irq_assign_handler(int irq, void (*handler)(system_state *sys));
void irq_remove_handler(int irq);
void isr_config();
void pic_config();
void irq_config();

#pragma region ISRs
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr128();
extern void isr177();

#pragma endregion ISRs

#pragma region IRQs
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
#pragma endregion IRQs

