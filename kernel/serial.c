#include "io.h"
#include "serial.h"
#include "stdlib/stdio.h"
#include "stdlib/string.h"

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
    while(serial_recevied(com) == 0);
    return inb(com);
}

void serial_write(unsigned int com, char* a){
    size_t i = 0;
    while(i < strlen(a)){
        while(serial_is_transmit_empty(com) == 0);
        outb(com, a[i]);
        i+=1;
    }
}

void debug_print(char *a) {
    serial_write(SERIAL_COM1_START, "[DEBUG]:");
    serial_write(SERIAL_COM1_START, a);
    serial_write(SERIAL_COM1_START, "\n");
}

void debug_print_hex(char *prefix, uint64_t val) {
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';
    buf[18] = '\0';
    
    char *hex_chars = "0123456789ABCDEF";
    for (int i = 15; i >= 0; i--) {
        buf[2 + i] = hex_chars[val & 0xF];
        val >>= 4;
    }
    
    debug_print(prefix);
    debug_print(buf);
}

void serial_init(unsigned short com){
    outb(com +1, 0x00);

    serial_config_brate(com, 2);
    serial_config_line(com);
    serial_config_buff(com);
    serial_config_modem(com);

    //serial_test_basic();
}
