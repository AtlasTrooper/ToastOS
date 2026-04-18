.set ALIGN, 1 <<0 
.set MEMINFO, 1 << 1//Mem + vid info 
.set FLAGS, ALIGN | MEMINFO 
.set MAGIC, 0x1BADB002 
.set CHECKSUM, -(MAGIC + FLAGS) 

.section .multiboot, "a", @progbits
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .boot

.global _start
.type _start, @function
_start:
  mov $(init_page_dir - 0xC0000000), %eax 
  mov %eax, %cr3
  
  mov %cr4, %ecx
  or %ecx, 0x00000010
  mov %ecx, %cr4

  mov %cr0, %ecx
  or %ecx, 0x80000000
  mov %ecx, %cr0
  
  lea higher_half, %eax
  jmp *%eax

.section .data
.align 4096
.global init_page_dir
init_page_dir:
  //PDE config [PS=1|D|A|PCD|PWT|U/S|R/W=1|P=1] -> 0x83
  .long 0x00000083
  .fill 767, 4, 0

  .long (0 << 22) | 0x83
  .long (1 << 22) | 0x83
  .long (2 << 22) | 0x83
  .long (3 << 22) | 0x83

  .fill 252, 4, 0

.section .text
.global higher_half
higher_half:
    mov $stack_top, %esp
    push %ebx
    xor %ebp, %ebp
    call kernel_main
    cli

halt:
  hlt
  jmp halt





  



