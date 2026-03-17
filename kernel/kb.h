
#include "io.h"
#include "vga.h"
#include "interrupts/idt.h"
#define KB_DATA 0x60
#define KB_COM 0x64
#define RELEASED 0x80

#define L_SHIFT 0x2A
#define R_SHIFT 0x36

#define REL_SHIFT_L 0xAA
#define REL_SHIFT_R 0xB6

#define CAPS_LOCK 0x3A

typedef struct {
    char name[16];
    uint8_t lower[128];
    uint8_t upper[128];
} keymap_t;

typedef enum SCANSET{
    ONE,
    TWO,
    THREE
}SCANSET;

void kb_handler(system_state *sys);
void kb_init();
