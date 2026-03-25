#include "kb.h"
#include "stdlib/stdio.h"

keymap_t def_layout = {
    "US-English",
    { 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ' },
    { 0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ' }
};

int caps_on;
int shift;
int numlock;

void kb_handler(system_state *sys){
    unsigned char scancode;

    scancode = inb(KB_DATA);

    if(scancode == L_SHIFT || scancode == R_SHIFT){
            shift = 1;
            //printf(" |SHIFT %s|\n", shift == 1? "ON" : "OFF");
    }
    else if(scancode == CAPS_LOCK){
            caps_on = !caps_on;
            //printf(" [CAPS %s]\n", caps_on == 1? "ON" : "OFF");
    }
    else if((scancode & RELEASED) && (scancode == REL_SHIFT_L || scancode == REL_SHIFT_R)){
            shift = 0;
            //printf(" |SHIFT %s|\n", shift == 1? "ON" : "OFF");
    }
    else if (!(scancode & RELEASED)){
        (shift ^ caps_on) ? printf("%c", def_layout.upper[scancode]) : printf("%c", def_layout.lower[scancode]);
    }
}

void kb_init(){
    caps_on = 0;
    shift = 0;
    irq_assign_handler(1, kb_handler);
    //printf("\nKEYBOARD INITIALIZED, CURRENT LAYOUT: %s \n", def_layout.name);
}

