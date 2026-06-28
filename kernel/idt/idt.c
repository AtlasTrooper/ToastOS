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

void kpanic(system_state *sys, const char *fmt, ...) {

    //Entering kernel panic, printing fmt message, reg state, stack trace
    asm("cli");

        printf("\n");
    printf("==========================================================\n");
    printf("                    *** KERNEL PANIC ***                  \n");
    printf("==========================================================\n");
 
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
 
    if (sys) {
        printf("----------------------------------------------------------\n");
        printf("  RAX=%016llx  RBX=%016llx\n", sys->rax, sys->rbx);
        printf("  RCX=%016llx  RDX=%016llx\n", sys->rcx, sys->rdx);
        printf("  RSI=%016llx  RDI=%016llx\n", sys->rsi, sys->rdi);
        printf("  RBP=%016llx  RSP=%016llx\n", sys->rbp, sys->rsp);
        printf("  R8 =%016llx  R9 =%016llx\n", sys->r8,  sys->r9);
        printf("  R10=%016llx  R11=%016llx\n", sys->r10, sys->r11);
        printf("  R12=%016llx  R13=%016llx\n", sys->r12, sys->r13);
        printf("  R14=%016llx  R15=%016llx\n", sys->r14, sys->r15);
        printf("----------------------------------------------------------\n");
        printf("  RIP=%016llx  CS =%016llx\n", sys->rip, sys->cs);
        printf("  RSP=%016llx  SS =%016llx\n", sys->rsp, sys->ss);
        printf("  RFLAGS=%016llx\n",            sys->rflags);
        printf("  Error code: %llx\n",           sys->error_code);
        printf("----------------------------------------------------------\n");
    }
 
    printf("System halted.\n");
    printf("==========================================================\n");
 
    for (;;) asm("hlt");
}

static void pfHandler(system_state *sys) {
    uint64_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r"(cr2));

    uint64_t ec = sys->error_code;

    printf("\n[PAGE FAULT]\n");
    printf("Faulting addr: %016llx\n", cr2);
    printf("RIP          : %016llx\n", sys->rip);
    printf("Cause        : %s %s %s %s %s\n",
    (ec & (1<<0)) ? "Protection violation" : "non-present page",
    (ec & (1<<1)) ? "write" : "read", 
    (ec & (1<<2)) ? "user" : "kernel", 
    (ec & (1<<3)) ? "reserved bit" : "", 
    (ec & (1<<4)) ? "NX-violation" : "");

    kpanic(sys, "Unhandled page fault at %016llx", cr2);

}

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
    
    if (sys->interr_num == 14) {
        pfHandler(sys);
        return;
    }
    
    if (sys->interr_num < 32) {
        kpanic(sys, "Exception: %s", excep_trace[sys->interr_num]);
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