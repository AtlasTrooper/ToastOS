#include "qol.h"
#include "vga.h"
#include "io.h"
//IO PORTS
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5

//IO port commands
#define FB_HIGH_COM 14
#define FB_LO_COM 15

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
u8 cursor_pos = 0;

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
  fb_move_cursor(index);
  
}

void newLine(){
  if(terminal_row < VGA_H -1){
    terminal_row +=1;
    terminal_col = 0; 
  }
  else{
    terminal_scroll();
    terminal_col = 0;
  }
}

void putchar(char c){
  switch(c){
    case '\n':
      newLine();
      break;
    case '\r':
      terminal_col = 0;
      break;
    case '\b':
      if(terminal_col == 0 && terminal_row !=0){
        terminal_row -=1;
        terminal_col = VGA_W;
      }
      terminal_putEntryAt(' ', terminal_color, --terminal_col, terminal_row);
      break;
    case '\t':
      if(terminal_col == VGA_W){
        newLine();
      }
      u16 tabAmount = 4-(terminal_col%4);
      while(tabAmount !=0){
        terminal_putEntryAt(' ', terminal_color, terminal_col++, terminal_row);
        tabAmount-=1;
      }
      break;

    default:
      if(terminal_col == VGA_W){
        newLine();
      }
      terminal_putEntryAt(c, terminal_color, terminal_col++, terminal_row);
      break;
  }    
}

void terminal_write(const char * data, size_t size){
  for(size_t i = 0; i < size; i ++){
    putchar(data[i]);
  }
}

void putstr(const char * data){
  terminal_write(data, strlen(data));
}

void terminal_scroll(){
  terminal_row = VGA_H-1;
  terminal_col = 0;
  // size_t start = 0;
  // terminal_buff[start] = vga_entry('A',terminal_color);
  for(int r = 0; r < VGA_H-1; r ++){
    for(int c = 0; c < VGA_W; c++){
      const size_t index_dst = r*VGA_W+c;
      const size_t index_src = ((r+1)*VGA_W)+c;
          
      terminal_buff[index_dst] = terminal_buff[index_src];  
    }
  }
  for(int c = 0; c < VGA_W; c++){
    const size_t index = (VGA_H-1)*VGA_W+c;
    terminal_buff[index] = vga_entry(' ', terminal_color);
  }
}

void terminal_clear(){
  terminal_init();
}

void fb_move_cursor(unsigned short pos){
  outb(FB_COMMAND_PORT, FB_HIGH_COM);
  outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
  outb(FB_COMMAND_PORT, FB_LO_COM);
  outb(FB_DATA_PORT, pos & 0x00FF);
}