#include "io.h"

//we use io to access/send serial data

/*each COM port region has several sections relative
to where it's data port starts. Additionally, each section, 
such as the FIFO port, can be divided into it's own subsections*/
#define SERIAL_COM1_START 0x3F8
#define SERIAL_COM2_START 0x2F8

#define SERIAL_COM3_START 0x3E8
#define SERIAL_COM4_START 0x2E8

#define SERIAL_DATA_PORT(base) (base)
#define SERIAL_FIFO_PORT(base) (base +2)
#define SERIAL_LINE_PORT(base) (base +3)
#define SERIAL_MODEM_PORT(base) (base +4)
#define SERIAL_LINE_STATUS_PORT(base) (base +5)


/*tells the serial port to expect HL since
 it expects 8 bits at a time. Here we provide the serial
 port with a rate and divisor*/
#define SERIAL_LINE_ENABLE_DLAB 0x080

void serial_config_brate(unsigned short com, unsigned short div){

    /*in order to config the brate, we send the following data
    to our desired COM port */

    //Expect High Lo
    outb(SERIAL_LINE_PORT(com), SERIAL_LINE_ENABLE_DLAB);

    //High data
    outb(SERIAL_DATA_PORT(com), (div >> 8)&(0x00FF));

    //Lo data
    outb(SERIAL_DATA_PORT(com), div & 0x00FF);
    
}

/*Data sending configs layout
Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
Content: | d | b | prty  | s | dl  |*/

void serial_config_line(unsigned short com){
    /*apparently 0x03 is a pretty standard config format
        meaning:
        8 bit length
        no parity bit, one stop bit, and break control disabled
    */
    outb(SERIAL_LINE_PORT(com), 0x03);
}

void serial_config_buff(unsigned short com){
    outb(SERIAL_FIFO_PORT(com), 0xC7);
}

void serial_config_modem(unsigned short com){
    outb(SERIAL_MODEM_PORT(com), 0x03);
}

int serial_is_transmit_empty(unsigned int com){
    //checks bit 5 of the line status to check if the transmit fifo queue is empty
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

int serial_recevied(unsigned int com){
    return inb(SERIAL_LINE_STATUS_PORT(com))&1;
}

char serial_read(unsigned int com){
    while(serial_recevied(com));
    return inb(com);
}

void serial_write(unsigned int com, char a){
    while(serial_is_transmit_empty(com) == 0);
    outb(com, a);
}