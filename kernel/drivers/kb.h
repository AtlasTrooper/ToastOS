#pragma once
#include <stdint.h>
#include "../util.h"
#include "../io.h"
#include "../idt/idt.h"
#include "../shell/tsh.h"

#define KB_DATA    0x60
#define KB_COM     0x64
#define RELEASED   0x80

#define L_CTRL     0x1D
#define REL_CTRL_L 0x9D
#define L_SHIFT    0x2A
#define R_SHIFT    0x36
#define REL_SHIFT_L 0xAA
#define REL_SHIFT_R 0xB6
#define CAPS_LOCK  0x3A
#define R_CTRL     0x1D
#define REL_CTRL_R 0x9D
#define UP_ARROW   0x48
#define DOWN_ARROW 0x50

#define KB_BUF_SIZE 256

typedef struct PACKED {
    char    name[16];
    uint8_t lower[128];
    uint8_t upper[128];
} keymap_t;

void kb_enqueue(char c);
int  kb_haschar(void);
char kb_getchar(void);
void kb_handler(system_state *sys);
void init_kb(void);