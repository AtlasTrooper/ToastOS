.global loadGDT
.global flushAndReload
loadGDT:
    mov 4(%esp), %eax //loads a ptr to our gdt struct, (ptr size is 4 bytes) into the eax register
    lgdt (%eax) //loads the eax register data into our gdt
    ret 

flushAndReload:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    ljmp $0x08, $flush_cs

flush_cs:
    ret
    
