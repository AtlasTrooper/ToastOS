ENTRY(_start)

SECTIONS
{

  . = 1M; /*Boot sections*/

  .multiboot BLOCK(4K): AT(ADDR(.multiboot)){
    *(.multiboot)
  }

  .boot BLOCK(4K): AT(ADDR(.boot)){
    *(.boot)
  }

  . = 0xC0100000; /*Higher half*/

  .text BLOCK(4K): AT(ADDR(.text) - 0xC0000000){
    *(.text)
  }

  .rodata BLOCK(4K): AT(ADDR(.rodata) - 0xC0000000){
    *(.rodata)
  }

  .data BLOCK(4K): AT(ADDR(.data) - 0xC0000000){
    *(.data)
  }

  .bss BLOCK(4K): AT(ADDR(.bss) - 0xC0000000){
    *(COMMON)
    *(.bss)
  }

  _kernel_end = .;
}
