
.set ALIGN, 1 <<0 
.set MEMINFO, 0x00000003//Mem + vid info 
.set FLAGS, ALIGN | MEMINFO 
.set MAGIC, 0x1BADB002 
.set CHECKSUM, -(MAGIC + FLAGS) 

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384 #16 Kib
stack_top:

//kernel entrypoint
.section .text
.global _start
.type _start, @function
_start:
  mov $stack_top, %esp
  push %ebx
  push %eax
/*Future updates, add GDT,IDT[done] and paging[w.i.p] here*/

  call kernel_main

  cli
1: hlt
  jmp 1b


/*Set the size of the _start symbol to the current location '.' minus it's start.
 Apparently this is useful when debugging or when implementing call tracing*/

.size _start, . - _start

  



