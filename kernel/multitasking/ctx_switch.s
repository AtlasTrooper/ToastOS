.global switch_to_task
.type switch_to_task, @function

.equ TCB_RSP, 16
.equ TCB_RSP0, 24
.equ TCB_CR3, 32

#C side function call: void switch_to_task(thread_t* next_thread);

#disable IRQ's before and enable them after
switch_to_task:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    #save prev task addr
    movq current_task_TCB(%rip), %rax # %rax = current tcb ptr addr
    movq %rsp, TCB_RSP(%rax)

    #next task
    movq %rdi, current_task_TCB(%rip)
    movq %rdi, %rsi

    #TSS update
    movq TCB_RSP0(%rsi), %rdi
    call load_tss

    #stack ptr switch to next task kernel stack
    movq TCB_RSP(%rsi), %rsp

    #comp vaddr space
    movq TCB_CR3(%rsi), %rax
    movq %cr3, %rcx
    cmpq %rcx, %rax
    je .doneVAS
    movq %rax, %cr3

.doneVAS:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    retq


