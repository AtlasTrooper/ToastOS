#include "qol.h"
enum vga_color {

  VGA_BLACK=0,
  VGA_BLUE=1,
  VGA_GREEN=2,
  VGA_CYAN=3,
  VGA_RED=4,
  VGA_MAGENTA=5,
  VGA_BROWN=6,
  VGA_LIGHT_GREY=7,
  VGA_DARK_GREY=8,
  VGA_LIGHT_BLUE=9,
  VGA_LIGHT_GREEN=10,
  VGA_LIGHT_CYAN=11,
  VGA_LIGHT_RED=12,
  VGA_lIGHT_MAGENTA=13,
  VGA_LIGHT_BROWN=14,
  VGA_WHITE=15,
};

// static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg);
// static inline u16 vga_entry(uchar uc, u8 color);

size_t strlen(const char * str);
void newLine();
void terminal_init(void);
void terminal_set_color(uint8_t color);
void terminal_putEntryAt(char c, uint8_t color, size_t x, size_t y);
void putchar(char c);
void terminal_write(const char * data, size_t size);
void putstr(const char * data);
void terminal_scroll();
void fb_move_cursor(unsigned short pos);

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg){

return fg | bg << 4;

}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
 return (uint16_t)uc | (uint16_t) color << 8;
}

