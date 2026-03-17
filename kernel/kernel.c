#include "qol.h"
#include "vga.h"
#include "serial.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "timer.h"
#include "kb.h"

#if defined(__linux__)
#error "You are not using your cross comp, go do that!"
#endif

#if !defined(__i386__)
#error "This os needs to be compiled with an ix86-elf comp"
#endif

void kernel_main(void){
  initGDT();
  initIDT();
  terminal_init();
  serial_init(SERIAL_COM1_START);
  init_timer();
  kb_init();
  //speaker_config(600);
  
  //asm volatile ("int $0");

  putstr("=================================================================\n");
  putstr("=Welcome to the Toast Operating System| est. 2026 Tomer Wiesel |=\n");
  putstr("=================================================================\n");

  asm volatile("sti"); while(1);
} 




