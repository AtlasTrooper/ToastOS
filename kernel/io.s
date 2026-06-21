.global outb
.global inb

# outb(uint16_t port, uint8_t data)
# port -> di, data -> si
outb:
    mov %di, %dx
    mov %si, %al
    out %al, %dx
    ret

# uint8_t inb(uint16_t port)
# port -> di, return value in al
inb:
    mov %di, %dx
    in %dx, %al
    ret