#include "tsh.h"

//max size of a single command
#define KBUF_SIZE 256

static char kbuf[KBUF_SIZE];
static volatile int kbuf_head = 0, kbuf_tail = 0; 


