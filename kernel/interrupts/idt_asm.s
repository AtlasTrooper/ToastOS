.global loadIDT
loadIDT:
    mov 4(%esp), %eax
    lidt (%eax)
    sti
    ret

#ISRs_______________________________

# Without error code
.macro NO_ERRORCODE_handler param
.global isr\param
    isr\param:
            cli
            pushl $0
            pushl $\param
            jmp isr_common_handler
.endm

#With error code
.macro ERRORCODE_handler param
.global isr\param
    isr\param:
        cli
        pushl $\param
        jmp isr_common_handler
.endm

NO_ERRORCODE_handler 0
NO_ERRORCODE_handler 1
NO_ERRORCODE_handler 2
NO_ERRORCODE_handler 3
NO_ERRORCODE_handler 4
NO_ERRORCODE_handler 5
NO_ERRORCODE_handler 6
NO_ERRORCODE_handler 7 #cause the code is 0

ERRORCODE_handler 8

NO_ERRORCODE_handler 9

ERRORCODE_handler 10
ERRORCODE_handler 11
ERRORCODE_handler 12
ERRORCODE_handler 13
ERRORCODE_handler 14

NO_ERRORCODE_handler 15
NO_ERRORCODE_handler 16

ERRORCODE_handler 17 #cause the code is 0

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

.extern isr_handler

isr_common_handler:
    pushal
    push %ds
    push %es
    push %fs
    push %gs
    
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov %esp, %eax
    push %eax
    call isr_handler

    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popal
    add $8, %esp
    
    iret


#IRQs___________________________

.macro IRQ_ param, vector
.global irq\param
    irq\param:
        cli
        pushl $0
        pushl $\vector
        jmp irq_common_handler
.endm

IRQ_ 0, 32
IRQ_ 1, 33
IRQ_ 2, 34
IRQ_ 3, 35
IRQ_ 4, 36
IRQ_ 5, 37
IRQ_ 6, 38
IRQ_ 7, 39
IRQ_ 8, 40
IRQ_ 9, 41
IRQ_ 10, 42
IRQ_ 11, 43
IRQ_ 12, 44
IRQ_ 13, 45
IRQ_ 14, 46
IRQ_ 15, 47


.extern irq_handler

irq_common_handler:
    pushal
    push %ds
    push %es
    push %fs
    push %gs
    
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov %esp, %eax
    push %eax
    call irq_handler

    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popal
    add $8, %esp
    
    iret
