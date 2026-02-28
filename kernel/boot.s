
.set ALIGN, 1 <<0 
.set MEMINFO, 1<<1 
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


/*Future updates, add GDT and paging here*/
  gdtr DW 0 ;
       DD 0 ;

  setGdt:
    MOV AX, [esp+4]
    MOV [gdtr], AX
    MOV EAX, [ESP+8]
    MOV [gdtr+2], EAX
    LGDT [gdtr]
    RET

  reloadSegments:
    /*Reload the CS register with code selector*/
    JMP 0x08:.gdt_flush // 0x08 represents the code seg
  
  //flush the data segment registers
  .gdt_flush:
    MOV AX, 0x10
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    MOV SS, AX
    RET

  call kernel_main

  cli
1: hlt
  jmp 1b


/*Set the size of the _start symbol to the current location '.' minus it's start.
 Apparently this is useful when debugging or when implementint call tracing*/

.size _start, . - _start

  



