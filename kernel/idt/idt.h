#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../util.h"
#include "../gdt/gdt.h"

#define GDT_CS          0x08
#define INT_GATE_FLAGS  0x8E

#define PIC1            0x20
#define PIC2            0xA0
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define EOI             0x20

typedef struct PACKED {
    uint16_t lim;
    uint64_t base;
} IDT;

typedef struct PACKED {
    uint16_t base_lo;
    uint16_t selector;
    uint8_t  ist;        // interrupt stack table index (0 = don't use)
    uint8_t  flags;
    uint16_t base_mid;
    uint32_t base_hi;
    uint32_t reserved;
} idt_entry;

typedef struct PACKED {
    uint64_t gs, fs, es, ds;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t interr_num, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} system_state;

/*Kernel panic, pending Ascii art update lol */
void kpanic(system_state *sys, const char *fmt, ...);

#define KPANIC(sys, fmt, ...) \
    kpanic(sys, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    
void initIDT(void);
void encode_interrupt_gate(uint32_t index, uint64_t base, uint16_t sel, uint8_t flags);
void isr_handler(system_state *sys);
void irq_handler(system_state *sys);
void irq_assign_handler(int irq, void (*handler)(system_state *sys));
void irq_remove_handler(int irq);
void isr_config(void);
void pic_config(void);
void irq_config(void);

extern void loadIDT(IDT *idt);

extern void isr0();  extern void isr1();  extern void isr2();  extern void isr3();
extern void isr4();  extern void isr5();  extern void isr6();  extern void isr7();
extern void isr8();  extern void isr9();  extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();
extern void isr128(); extern void isr177();

extern void irq0();  extern void irq1();  extern void irq2();  extern void irq3();
extern void irq4();  extern void irq5();  extern void irq6();  extern void irq7();
extern void irq8();  extern void irq9();  extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();