.section .data
.align 4

.global gdt
.global gdtr

gdtr:
    .word 0          # limit
    .long 0          # base


.section .text
.global setGdt
.type setGdt, @function

setGdt:
    movw 4(%esp), %ax
    movw %ax, gdtr

    movl 8(%esp), %eax
    movl %eax, gdtr+2

    lgdt gdtr
    ret


.global reloadSegments
.type reloadSegments, @function

reloadSegments:
    # Far jump to reload CS
    ljmp $0x08, $gdt_flush


gdt_flush:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ret