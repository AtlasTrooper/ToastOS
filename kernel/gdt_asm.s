.global load_gdt
.global flush
.global load_tss

load_gdt:
    mov 4(%esp), %eax
    lgdt (%eax)
    ret

flush:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $reload
    

reload:
    ret


load_tss:
    mov $0x28, %ax
    ltr %ax
    ret
    
