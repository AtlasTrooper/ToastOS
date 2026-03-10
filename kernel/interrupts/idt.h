#include "../qol.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

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

typedef struct PACKED cpu_state{

    uint32_t cr2;
    uint32_t ds;

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esp;    
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

}cpu_state;

typedef struct PACKED stack_state{
    uint32_t interr_num;

    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
}stack_state;

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

void isr_handler(
    cpu_state *cpu,
    stack_state *stack,
);

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


void initIDT();
void encode_interrupt_gate(uint32_t index, uint32_t base, uint16_t sel, uint8_t flags);
