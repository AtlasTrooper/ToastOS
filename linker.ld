ENTRY(_start)

SECTIONS
{

  . = 1M; /*Boot sections*/

  .multiboot : AT(ADDR(.multiboot)){
    KEEP(*(.multiboot))
  }

  .boot : AT(ADDR(.boot)){
    *(.boot)
  }

  . = ALIGN(4K);

  . = . + 0xC0000000;

  _kernel_v_start = .;
  _kernel_p_start = . - 0xC0000000;

  .text BLOCK(4K): AT(ADDR(.text) - 0xC0000000){
    *(.text)
  }

  .rodata BLOCK(4K): AT(ADDR(.rodata) - 0xC0000000){
    *(.rodata)
    *(.rodata.*)
    *(.rodata1) 
  }

  .data BLOCK(4K): AT(ADDR(.data) - 0xC0000000){
    *(.data)
  }

  .bss BLOCK(4K): AT(ADDR(.bss) - 0xC0000000){
    *(COMMON)
    *(.bss)
  }

  /DISCARD/ :
  {
    *(.comment)
    *(.eh_frame)
    *(.note.*)
  }

  
  _kernel_v_end = .;
  _kernel_p_end = . - 0xC0000000;
}
