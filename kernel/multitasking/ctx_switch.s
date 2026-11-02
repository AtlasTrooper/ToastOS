.global context_switch
.global task_first_run
.type context_switch, @function

.extern current_task_TCB
.extern syscall_kernel_rsp
.extern load_tss
.extern update_task_time
.extern kernel_task_startup
.extern task_exit

# offsets into task_t (pinned by _Static_asserts in task.h)
.equ TCB_RSP, 16
.equ TCB_RSP0, 24
.equ TCB_CR3, 32

# C side: void context_switch(task_t* next_task);
# IRQs are already disabled by the caller (lock_schedule / lock_stuff).
context_switch:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # save prev task's stack pointer
    movq current_task_TCB(%rip), %rax
    movq %rsp, TCB_RSP(%rax)

    # keep `next` in r12 cause callee-saved
    # r12 was just saved above and gets restored from the new stack, so it's free to use.
    movq %rdi, %r12

    # rsp is 8 mod 16 here, gotta align
    subq $8, %rsp

    # update elapsed time to the task that was running, THEN switch current
    call update_task_time

    movq %r12, current_task_TCB(%rip)

    # kernel stack the CPU uses for ring3->ring0 transitions:
    #  - interrupts/exceptions via TSS.rsp0
    #  - `syscall` has no TSS lookup, syscall_entry reads syscall_kernel_rsp
    movq TCB_RSP0(%r12), %rdi
    movq %rdi, syscall_kernel_rsp(%rip)
    call load_tss

    # switch to next task's kernel stack
    movq TCB_RSP(%r12), %rsp

    # switch address space only if dif
    movq TCB_CR3(%r12), %rax
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

# First thing every new task executes (context_switch `ret`s here).
# On entry: rsp is 16-byte aligned, r12 = entry fn, r13 = arg0, r14 = arg1
task_first_run:
    call kernel_task_startup     # unlock_schedule(), syncs with lock across switch
    movq %r13, %rdi
    movq %r14, %rsi
    call *%r12                   # kernel task body, or user_task_launcher(entry, user_rsp)
    call task_exit               # the task function returned: terminate it
    ud2
