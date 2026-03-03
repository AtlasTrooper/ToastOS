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
#define SERIAL_LINE_ENABLE_DLAB 0x80

void serial_config_brate(unsigned short com, unsigned short div);
void serial_config_line(unsigned short com);
void serial_config_buff(unsigned short com);
void serial_config_modem(unsigned short com);
int serial_is_transmit_empty(unsigned int com);
int serial_recevied(unsigned int com);
char serial_read(unsigned int com);
void serial_write(unsigned int com, char* a);
void serial_test_basic();
void serial_init(unsigned short com);





