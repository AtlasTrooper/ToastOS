#include "qol.h"
#include "vga.h"
#include "serial.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "timer.h"
#include "kb.h"
#include "memoryMan/multiboot.h"
#include "memoryMan/memory.h"

#if defined(__linux__)
#error "You are not using your cross comp, go do that!"
#endif

#if !defined(__i386__)
#error "This os needs to be compiled with an ix86-elf comp"
#endif

void kernel_main(uint32_t magicNum, multiboot_info* boot_data){
  initGDT();
  initIDT();
  terminal_init();
  serial_init(SERIAL_COM1_START);
  init_timer();
  
  //speaker_config(600);
  //asm volatile ("int $0");

  putstr("=================================================================\n");
  putstr("Welcome to the Toast Operating System| est. 2026 Tomer Wiesel    \n(eden was here :)\n");
  putstr("=================================================================\n");
  
  kb_init();

  //initMem(boot_data);

  asm volatile("sti"); while(1);
} 




