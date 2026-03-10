#include "qol.h"
#include "vga.h"
#include "serial.h"
#include "gdt.h"

#if defined(__linux__)
#error "You are not using your cross comp, go do that!"
#endif

#if !defined(__i386__)
#error "This os needs to be compiled with an ix86-elf comp"
#endif




void kernel_main(void){
  initGDT();
  terminal_init();
  serial_init(SERIAL_COM1_START);
  putstr("=======================================\n");
  putstr("=Welcome to the Toast Operating System=\n");
  putstr("=======================================\n");
} 




