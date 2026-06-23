#pragma once
#include <stdint.h>
#include <stddef.h>

#define CON_BLACK        0x00000000
#define CON_WHITE        0x00FFFFFF
#define CON_LIGHT_GREY   0x00AAAAAA
#define CON_DARK_GREY    0x00555555
#define CON_RED          0x00FF5555
#define CON_LIGHT_GREEN  0x0055FF55
#define CON_GREEN        0x0000AA00
#define CON_LIGHT_BLUE   0x005555FF
#define CON_BLUE         0x000000AA
#define CON_CYAN         0x0055FFFF
#define CON_MAGENTA      0x00FF55FF
#define CON_YELLOW       0x00FFFF55
#define CON_LIGHT_BROWN  0x00FFAA00
#define CON_BROWN        0x00AA5500

void console_init(void);
void console_set_color(uint32_t fg, uint32_t bg);
void putchar(char c);
void putstr(const char *str);
void newLine(void);
void clearLine(size_t row);
void clearCurrentLine(void);
void terminal_clear(void);
void terminal_scroll(void);
void console_tick(void);