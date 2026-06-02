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

.section .boot, "a", @progbits

.global _start
.type _start, @function
_start:
  //movw $0x0F31, (0xB8000)
  mov $(page_directory - 0xC0000000), %ecx 
  mov %ecx, %cr3
  
  mov %cr4, %ecx
  or $0x00000010, %ecx
  mov %ecx, %cr4

  mov %cr0, %ecx
  or $0x80000000, %ecx
  mov %ecx, %cr0
  //movw $0x0F32, (0xB8002)
  lea higher_half, %ecx
  jmp *%ecx

.section .text
.global higher_half
.type higher_half, @function
higher_half:
    mov $stack_top, %esp
    push %ebx //contains the multiboot header
    push %eax
    xor %ebp, %ebp //no stack frame before the higher half
    call kernel_main
    cli

halt:
  hlt
  jmp halt

.section .data
.align 4096
.global page_directory
page_directory:
  //PDE config [PS=1|D|A|PCD|PWT|U/S|R/W=1|P=1] -> 0x83
  
  .long 0x83
  .fill 767, 4, 0

  .long (0 << 22) | 0x83
  .long (1 << 22) | 0x83
  .long (2 << 22) | 0x83
  .long (3 << 22) | 0x83

  .fill 251, 4, 0
  .long 0





  



