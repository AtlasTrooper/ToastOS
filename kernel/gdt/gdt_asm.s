.global gdt_flush
.global tss_flush

gdt_flush:
    lgdt (%rdi)

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    pop %rdi
    push $0x08
    push %rdi
    lretq

tss_flush:
    mov $0x28, %ax
    ltr %ax
    ret