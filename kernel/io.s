.global outb 
.global inb
.global serial_putc
//outb will send a byte to an io port

/*
stack:[esp + 8] the data byte
      [esp +4] the IO port
      [esp] return address
*/

outb:
    mov 8(%esp), %al
    mov 4(%esp), %dx
    out %al, %dx
    ret

/*inb will send a byte to the serial port and return it's data*/

inb:
      mov 4(%esp), %dx
      in %dx, %al
      ret

serial_putc:
    push %ebp
    mov %esp, %ebp

    mov 8(%ebp), %al   # char
    mov $0x3F8, %dx
    out %al, %dx

    pop %ebp
    ret