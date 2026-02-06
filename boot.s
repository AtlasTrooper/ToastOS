/*
MultiBoot Constants
*/

.set ALIGN, 1 <<0 //aligns loaded modules on page boundaries
.set MEMINFO, 1<<1 // provides our memory map
.set FLAGS, ALIGN | MEMINFO /* this is the multiboot 'flag' field*/
.set MAGIC, 0x1BADB002 //This is what the bootloader will look for
.set CHECKSUM, -(MAGIC + FLAGS) //checksum of above to prove this is indeed the multiboot

/*Declare multiboot standard compatible header which will mark our program as the kernel. The bootloader will search for this signature in the first 8Kib of the kernel file, aligned at a 32 bit boundary. This signature is in it's own section so the header can be forced within the firs 8Kib of the kernel file*/

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*SP is not defined in multiboot standard so the kernel
must provide a stack. on x86 the stack grows downward according to the system v abi standard and is also 16 byte aligned*/

/*bss is our uninitialized static data that has yet to 
be assigned a value*/

.section .bss
.align 16
stack_bottom:
.skip 16384 #16 Kib
stack_top:

/*entry point to the kernel, from here we will enter 
32 bit protected mode and initialize the stack. We do this in asm
since C cannot function without a stack*/

.section .text
.global _start
.type _start, @function
_start:
  mov $stack_top, %esp

  /*Here we also initialize crucial processor state before the high level kernel is entered. Best to minimize early environment here as crucial features are offline. Our processor is not fully initialized here: floating point, instruction set extensions are not initialized yet. The GDT should be loaded here. Paging should be enabled here.*/

/*Future updates, add GDT*/

  /*After that, we enter the high level kernel! We've already aligned the stack so we're good to go*/

  call kernel_main

  /*If the system has nothing more to do, enter an infinite loop. To do that we :

  1. Disable interrupts with cli(usually we would need to clear interrupt enable in eflags but the bootloader already disables them)
  
  n o t e to setlf: says that you might later enable interrupts and return from kernel_main (apparently this is nonsensical to do)

  2.
  Wait for the next interrupt to arrive with hlt (halt instruction)
which will lock the computer as we disabled interrupts
  
  3. Finally, jump to the hlt instruction if it ever wakes up for some reason
  */

  cli
1: hlt
  jmp 1b


/*Set the size of the _start symbol to the current location '.' minus it's start. Apparently this is useful when debugging or when implementint call tracing*/

.size _start, . - _start

  



