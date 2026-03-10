#include "idt.h"
#include "../io.h"
#include "../serial.h"

#define GDT_CS 0x08
#define INT_GATE_FLAGS 0x8E

IDT idt;
idt_entry ent[256];
cpu_state cpu;
stack_state stack;


void initIDT(){
    idt.base = (uint32_t)&idt;
    idt.lim = (256*sizeof(idt_entry))-1;
    memset(&idt, 0, sizeof(idt_entry)*256);

    #pragma region PIC_CONFIG
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    oitb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, 0x0);
    oubt(PIC2_DATA, 0x0);
    #pragma endregion PIC_CONFIG

    #pragma region ISRs
    encode_interrupt_gate(0, (uint32_t)isr0,GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(1, (uint32_t)isr1,GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(2, (uint32_t)isr2,GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(3, (uint32_t)isr3,GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(4, (uint32_t)isr4, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(5, (uint32_t)isr5, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(6, (uint32_t)isr6, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(7, (uint32_t)isr7, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(8, (uint32_t)isr8, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(9, (uint32_t)isr9, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(10, (uint32_t)isr10, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(11, (uint32_t)isr11, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(12, (uint32_t)isr12, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(13, (uint32_t)isr13, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(14, (uint32_t)isr14, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(15, (uint32_t)isr15, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(16, (uint32_t)isr16, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(17, (uint32_t)isr17, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(18, (uint32_t)isr18, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(19, (uint32_t)isr19, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(20, (uint32_t)isr20, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(21, (uint32_t)isr21, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(22, (uint32_t)isr22, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(23, (uint32_t)isr23, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(24, (uint32_t)isr24, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(25, (uint32_t)isr25, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(26, (uint32_t)isr26, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(27, (uint32_t)isr27, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(28, (uint32_t)isr28, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(29, (uint32_t)isr29, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(30, (uint32_t)isr30, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(31, (uint32_t)isr31, GDT_CS, INT_GATE_FLAGS);

    //These are used for sys calls
    encode_interrupt_gate(128, (uint32_t)isr128, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(177, (uint32_t)isr177, GDT_CS, INT_GATE_FLAGS);


    #pragma endregion ISRs
    
    
    loadIDT((uint32_t)&idt);
    //encode all of our interrupts
    debug_print("INTERRUPTS LOADED AND READY");
}

void encode_interrupt_gate(uint32_t index, uint32_t base, uint16_t sel, uint8_t flags){
    ent[index].base_low = (base&0xFFFF);
    ent[index].sel = sel;
    ent[index].zero = 0;
    ent[index].flags = flags | 0x60;
    ent[index].base_high = ((base >> 16)&0xFFFF);
}

void isr_handler(cpu_state *cpu, stack_state *stack){
    if(stack->interr_num < 32){
        putstr(excep_trace[stack->interr_num]);
        putstr("\n");
        putstr("[ERROR]: Halting System");
        while(1);
    }
}