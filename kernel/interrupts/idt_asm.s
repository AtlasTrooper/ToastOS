.global loadIDT

loadIDT:
    mov 4(%esp), %eax
    lidt (%eax)
    ret


