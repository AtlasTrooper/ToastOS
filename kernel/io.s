.global outb 
.global inb
//outb will send a byte to an io port

/*
stack:[esp + 8] the data byte
      [esp +4] the IO port
      [esp] return address
*/

outb:
    movw 8(%esp), %al
    movw 4(%esp), %dx
    out %al, %dx
    ret

/*inb will send a byte to the serial port and return it's data*/

inb:
      movw 4(%esp), %dx
      in %dx, %al
      ret

