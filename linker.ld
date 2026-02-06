/*The bootloader will look at this image and start execution at the symbol designated as the entry point*/

ENTRY(_start)

/*The following section tells where the various sections of the obj files will be put into the final kernel img*/

SECTIONS
{

  /*we use the recommended 2MB offset since UEFI is cool like that(we don't get no fancy multiboot2 features so)*/

  . = 2M;

  /*ELF sections lets goo*/

  .text BLOCK(4K) : ALIGN(4K)
  {
    *(.multiboot)
    *(.text)
  }

  .rodata BLOCK(4K) : ALIGN(4K)
  {
    *(.rodata)
  }

  .data BLOCK(4K): ALIGN(4K)
  {
    *(.data)
  }

  .bss BLOCK(4K) : ALIGN(4K)
  {
    *(COMMON)
    *(.bss)
  }
    
  /*might add more stuff here later but for now dis good*/



}
