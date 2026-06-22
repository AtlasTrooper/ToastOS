#include "idt.h"
#include "../io.h"
#include "../serial.h"
#include "../stdlib/stdio.h"

static IDT       idtr;
static idt_entry idt_ent[256];

static void *irq_map[16] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

const char *excep_trace[] = {
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
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved"
};

void encode_interrupt_gate(uint32_t index, uint64_t base, uint16_t sel, uint8_t flags) {
    idt_ent[index].base_lo  = base & 0xFFFF;
    idt_ent[index].selector = sel;
    idt_ent[index].ist      = 0;
    idt_ent[index].flags    = flags;
    idt_ent[index].base_mid = (base >> 16) & 0xFFFF;
    idt_ent[index].base_hi  = (base >> 32) & 0xFFFFFFFF;
    idt_ent[index].reserved = 0;
}

void isr_handler(system_state *sys) {
    if (sys->interr_num < 32) {
        printf("\n[EXCEPTION] %s\n", excep_trace[sys->interr_num]);
        printf("  RIP: %llx  CS: %llx  RFLAGS: %llx\n",
               sys->rip, sys->cs, sys->rflags);
        printf("  RSP: %llx  SS: %llx\n", sys->rsp, sys->ss);
        printf("  Error code: %llx\n", sys->error_code);
        printf("[HALT]\n");
        for (;;) asm volatile("hlt");
    }
}

void irq_handler(system_state *sys) {
    void (*handler)(system_state *sys);
    handler = irq_map[sys->interr_num - 32];
    if (handler) handler(sys);

    if (sys->interr_num >= 40)
        outb(PIC2_COMMAND, EOI);
    outb(PIC1_COMMAND, EOI);
}

void irq_assign_handler(int irq, void (*handler)(system_state *sys)) {
    irq_map[irq] = handler;
}

void irq_remove_handler(int irq) {
    irq_map[irq] = 0;
}

void pic_config(void) {
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    outb(PIC1_DATA, 0x20);   // remap PIC1 to IRQ 32-39
    outb(PIC2_DATA, 0x28);   // remap PIC2 to IRQ 40-47

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
}

void isr_config(void) {
    encode_interrupt_gate(0,  (uint64_t)isr0,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(1,  (uint64_t)isr1,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(2,  (uint64_t)isr2,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(3,  (uint64_t)isr3,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(4,  (uint64_t)isr4,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(5,  (uint64_t)isr5,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(6,  (uint64_t)isr6,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(7,  (uint64_t)isr7,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(8,  (uint64_t)isr8,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(9,  (uint64_t)isr9,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(10, (uint64_t)isr10, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(11, (uint64_t)isr11, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(12, (uint64_t)isr12, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(13, (uint64_t)isr13, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(14, (uint64_t)isr14, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(15, (uint64_t)isr15, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(16, (uint64_t)isr16, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(17, (uint64_t)isr17, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(18, (uint64_t)isr18, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(19, (uint64_t)isr19, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(20, (uint64_t)isr20, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(21, (uint64_t)isr21, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(22, (uint64_t)isr22, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(23, (uint64_t)isr23, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(24, (uint64_t)isr24, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(25, (uint64_t)isr25, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(26, (uint64_t)isr26, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(27, (uint64_t)isr27, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(28, (uint64_t)isr28, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(29, (uint64_t)isr29, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(30, (uint64_t)isr30, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(31, (uint64_t)isr31, GDT_CS, INT_GATE_FLAGS);

    encode_interrupt_gate(128, (uint64_t)isr128, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(177, (uint64_t)isr177, GDT_CS, INT_GATE_FLAGS);
}

void irq_config(void) {
    encode_interrupt_gate(32, (uint64_t)irq0,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(33, (uint64_t)irq1,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(34, (uint64_t)irq2,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(35, (uint64_t)irq3,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(36, (uint64_t)irq4,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(37, (uint64_t)irq5,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(38, (uint64_t)irq6,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(39, (uint64_t)irq7,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(40, (uint64_t)irq8,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(41, (uint64_t)irq9,  GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(42, (uint64_t)irq10, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(43, (uint64_t)irq11, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(44, (uint64_t)irq12, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(45, (uint64_t)irq13, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(46, (uint64_t)irq14, GDT_CS, INT_GATE_FLAGS);
    encode_interrupt_gate(47, (uint64_t)irq15, GDT_CS, INT_GATE_FLAGS);
}


void initIDT(void) {
    idtr.base = (uint64_t)&idt_ent;
    idtr.lim  = (256 * sizeof(idt_entry)) - 1;

    for (int i = 0; i < 256; i++) {
        idt_ent[i].base_lo  = 0;
        idt_ent[i].selector = 0;
        idt_ent[i].ist      = 0;
        idt_ent[i].flags    = 0;
        idt_ent[i].base_mid = 0;
        idt_ent[i].base_hi  = 0;
        idt_ent[i].reserved = 0;
    }

    pic_config();
    isr_config();
    irq_config();

    loadIDT(&idtr);
    debug_print("IDT loaded\n");
}