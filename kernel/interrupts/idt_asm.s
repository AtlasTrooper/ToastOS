.global loadIDT
loadIDT:
    mov 4(%esp), %eax
    lidt (%eax)
    //do I need to call sti here?
    ret

//do I need to cli in the macros?

//no error code
.macro NO_ERRORCODE_handler param
.global interrupt_handler_ param
interrupt_handler_ param:
    pushl $0
    pushl \param
    jmp common_handler
.endm

//error code
.macro ERRORCODE_handler param
.global interrupt_handler_ param
interrupt_handler_ param:
    pushl \param
    jmp common_handler
.endm

NO_ERRORCODE_handler 0
NO_ERRORCODE_handler 1
NO_ERRORCODE_handler 2
NO_ERRORCODE_handler 3
NO_ERRORCODE_handler 4
NO_ERRORCODE_handler 5
NO_ERRORCODE_handler 6
NO_ERRORCODE_handler 7 //cause the code is 0

ERRORCODE_handler 8

NO_ERRORCODE_handler 9

ERRORCODE_handler 10
ERRORCODE_handler 11
ERRORCODE_handler 12
ERRORCODE_handler 13
ERRORCODE_handler 14

NO_ERRORCODE_handler 15
NO_ERRORCODE_handler 16

ERRORCODE_handler 17 //cause the code is 0

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

common_handler:
    pushal
    mov (%ds), %eax
    push %eax
    mov (%cr2), %eax
    push %eax

    mov $0x10, %ax
    mov (%ax), %ds
    mov (%ax), %es
    mov (%ax), %fs
    mov (%ax), %gs

    push %esp 

    call isr_handler

    add $8, %esp
    pop %ebx
    mov (%bx), %ds
    mov (%bx), %es
    mov (%bx), %fs
    mov (%bx), %gs

    popal
    add $8, %esp
    //Sti?
    iret

