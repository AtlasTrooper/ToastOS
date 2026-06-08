#include "qol.h"
#include "vga.h"
#include "serial.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "timer.h"
#include "kb.h"
#include "memoryMan/multiboot.h"
#include "memoryMan/memory.h"
#include "stdlib/malloc.h"
#include "shell/tsh.h"

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

  kb_init();

  initMem(boot_data);
  
  asm volatile("sti");

  init_shell();


} 




