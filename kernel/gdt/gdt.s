  global gdt
  
  gdtr DW 0 ;
       DD 0 ;

  setGdt:
    mov AX, [esp+4]
    mov [gdtr], AX
    mov EAX, [ESP+8]
    mov [gdtr+2], EAX
    LGDT [gdtr]
    RET

  reloadSegments:
    /*Reload the CS register with code selector*/
    JMP 0x08:.gdt_flush // 0x08 represents the code seg
  
  //flush the data segment registers
  .gdt_flush:
    mov AX, 0x10
    mov DS, AX
    mov ES, AX
    mov FS, AX
    mov GS, AX
    mov SS, AX
    RET
