#include "qol.h"

#if defined(__linux__)
#error "You are not using your cross comp, go do that!"
#endif

#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf comp"
#endif

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

static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg){

return fg | bg << 4;

}

static inline u16 vga_entry(uchar uc, u8 color)
{
 return (u16)uc | (u16) color << 8;
}

size_t strlen(const char * str){
  size_t len = 0;
  while(str[len]){
    len+=1;
  }
  return len;

}

#define VGA_W 80
#define VGA_H 25
#define VGA_MEM 0xB8000

size_t terminal_row;
size_t terminal_col;
u8 terminal_color;
u16* terminal_buff = (u16*)VGA_MEM;

void terminal_init(void){
  
  terminal_row = 0;
  terminal_col = 0;
  terminal_color = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
  for(size_t h=0; h<VGA_H; h++){//height is our y coord
    for(size_t w=0; w<VGA_W; w++){//width is our x coord
      const size_t index = h * VGA_W + w;
      terminal_buff[index] = vga_entry(' ', terminal_color);
    }
  }

}

void terminal_set_color(u8 color){
  terminal_color = color;
}

void terminal_putEntryAt(char c, u8 color, size_t x, size_t y)
{
  const size_t index = y * VGA_W + x;
  terminal_buff[index] = vga_entry(c,color);

}

void terminal_putchar(char c){

  terminal_putEntryAt(c, terminal_color, terminal_col, terminal_row);
  
  if(c == '\n'){terminal_row+=1; terminal_col = 0;}

  if(++terminal_col==VGA_W){
    terminal_col = 0;
    if(++terminal_row == VGA_H){
      terminal_row = 0;
    }
  }

}

void terminal_write(const char * data, size_t size){
  for(size_t i = 0; i < size; i ++){
    terminal_putchar(data[i]);
  }
}



void terminal_writeString(const char * data){
  terminal_write(data, strlen(data));
}

void kernel_main(void){

  terminal_init();
  terminal_writeString("Hello, Vsauce, Tomer here\n22");

}




