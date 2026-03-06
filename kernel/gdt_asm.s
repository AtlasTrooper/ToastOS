.global load_gdt
.global flush_n_reload
.global load_tss

load_gdt:
    mov 4(%esp), %eax //loads a ptr to our gdt struct, (ptr size is 4 bytes) into the eax register
    lgdt (%eax) //loads the eax register data into our gdt
    ret 

flush_n_reload:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ljmp $0x08, $flush_cs

flush_cs:
    ret

load_tss:
    mov $0x28, %ax
    ltr %ax
    ret



    
