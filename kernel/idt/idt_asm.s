.global loadIDT

loadIDT:
    lidt (%rdi)
    sti
    ret

# ─── Macros ──────────────────────────────────────────────────────────────────

.macro NO_ERRORCODE_handler num
.global isr\num
isr\num:
    cli
    pushq $0            # dummy error code
    pushq $\num
    jmp isr_common_handler
.endm

.macro ERRORCODE_handler num
.global isr\num
isr\num:
    cli
    pushq $\num
    jmp isr_common_handler
.endm

# ─── ISR stubs ───────────────────────────────────────────────────────────────

NO_ERRORCODE_handler 0
NO_ERRORCODE_handler 1
NO_ERRORCODE_handler 2
NO_ERRORCODE_handler 3
NO_ERRORCODE_handler 4
NO_ERRORCODE_handler 5
NO_ERRORCODE_handler 6
NO_ERRORCODE_handler 7
ERRORCODE_handler    8
NO_ERRORCODE_handler 9
ERRORCODE_handler    10
ERRORCODE_handler    11
ERRORCODE_handler    12
ERRORCODE_handler    13
ERRORCODE_handler    14
NO_ERRORCODE_handler 15
NO_ERRORCODE_handler 16
ERRORCODE_handler    17
NO_ERRORCODE_handler 18
NO_ERRORCODE_handler 19
NO_ERRORCODE_handler 20
NO_ERRORCODE_handler 21
NO_ERRORCODE_handler 22
NO_ERRORCODE_handler 23
NO_ERRORCODE_handler 24
NO_ERRORCODE_handler 25
NO_ERRORCODE_handler 26
NO_ERRORCODE_handler 27
NO_ERRORCODE_handler 28
NO_ERRORCODE_handler 29
NO_ERRORCODE_handler 30
NO_ERRORCODE_handler 31
NO_ERRORCODE_handler 128
NO_ERRORCODE_handler 177

# ─── ISR common handler ───────────────────────────────────────────────────────

.extern isr_handler
isr_common_handler:
    # Save general purpose registers
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Save segment registers
    mov %ds, %rax
    pushq %rax
    mov %es, %rax
    pushq %rax
    mov %fs, %rax
    pushq %rax
    mov %gs, %rax
    pushq %rax

    # Load kernel data segment
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Pass pointer to saved state as first argument
    mov %rsp, %rdi
    call isr_handler

    # Restore segment registers
    popq %rax
    mov %rax, %gs
    popq %rax
    mov %rax, %fs
    popq %rax
    mov %rax, %es
    popq %rax
    mov %rax, %ds

    # Restore general purpose registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax

    # Clean up error code and interrupt number
    addq $16, %rsp
    sti
    iretq

# ─── IRQ stubs ───────────────────────────────────────────────────────────────

.macro IRQ_ num, vector
.global irq\num
irq\num:
    cli
    pushq $0
    pushq $\vector
    jmp irq_common_handler
.endm

IRQ_ 0,  32
IRQ_ 1,  33
IRQ_ 2,  34
IRQ_ 3,  35
IRQ_ 4,  36
IRQ_ 5,  37
IRQ_ 6,  38
IRQ_ 7,  39
IRQ_ 8,  40
IRQ_ 9,  41
IRQ_ 10, 42
IRQ_ 11, 43
IRQ_ 12, 44
IRQ_ 13, 45
IRQ_ 14, 46
IRQ_ 15, 47

# ─── IRQ common handler ───────────────────────────────────────────────────────

.extern irq_handler
irq_common_handler:
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    mov %ds, %rax
    pushq %rax
    mov %es, %rax
    pushq %rax
    mov %fs, %rax
    pushq %rax
    mov %gs, %rax
    pushq %rax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov %rsp, %rdi
    call irq_handler

    popq %rax
    mov %rax, %gs
    popq %rax
    mov %rax, %fs
    popq %rax
    mov %rax, %es
    popq %rax
    mov %rax, %ds

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax

    addq $16, %rsp
    sti
    iretq