.global init_syscall
.global syscall_entry
.global enter_usermode
.global syscall_kernel_rsp
.extern syscall_handler

.section .bss
.align 8
# Kernel stack top of the CURRENT task. context_switch refreshes it on every switch
# (next to the TSS rsp0 update), so syscalls always land on the calling task's own
# kernel stack and can block / be preempted like any other kernel code.
syscall_kernel_rsp:
    .skip 8
saved_user_rsp:
    .skip 8

.section .text

# void init_syscall(void)
# Call once at boot, after the GDT is loaded.
# GDT layout this assumes (STAR below):
#   0x00 null
#   0x08 kernel code (64-bit)      <- SYSCALL loads CS = 0x08
#   0x10 kernel data               <- SYSCALL loads SS = 0x10
#   0x18 user data  (DPL 3)        <- SYSRET loads SS = 0x18 | 3
#   0x20 user code  (DPL 3, L=1)   <- SYSRET loads CS = 0x20 | 3
init_syscall:
    # EFER.SCE = 1 (enables syscall/sysret)
    movl $0xc0000080, %ecx
    rdmsr
    orl $1, %eax
    wrmsr

    # STAR: [47:32] = kernel CS base (0x08), [63:48] = sysret base (0x10)
    movl $0xc0000081, %ecx
    xorl %eax, %eax
    movl $0x00100008, %edx
    wrmsr

    # LSTAR: 64-bit syscall entry point
    movl $0xc0000082, %ecx
    leaq syscall_entry(%rip), %rax
    movq %rax, %rdx
    shrq $32, %rdx
    wrmsr

    # SFMASK: RFLAGS bits cleared on syscall entry (TF=0x100, IF=0x200, DF=0x400)
    movl $0xc0000084, %ecx
    movl $0x700, %eax
    xorl %edx, %edx
    wrmsr

    ret

# void enter_usermode(uint64_t rip /*rdi*/, uint64_t rsp /*rsi*/)  -- never returns
# Used for a task's FIRST trip to ring3 (sysret is only for returning from a syscall).
enter_usermode:
    cli
    pushq $0x1B          # SS  = user data | RPL3
    pushq %rsi           # RSP
    pushq $0x202         # RFLAGS: IF=1 + reserved bit 1
    pushq $0x23          # CS  = user code | RPL3
    pushq %rdi           # RIP

    # don't leak kernel register contents to ring3
    xorl %eax, %eax
    xorl %ebx, %ebx
    xorl %ecx, %ecx
    xorl %edx, %edx
    xorl %esi, %esi
    xorl %edi, %edi
    xorl %ebp, %ebp
    xorl %r8d, %r8d
    xorl %r9d, %r9d
    xorl %r10d, %r10d
    xorl %r11d, %r11d
    xorl %r12d, %r12d
    xorl %r13d, %r13d
    xorl %r14d, %r14d
    xorl %r15d, %r15d
    iretq

# Syscall ABI (same as Linux):
#   rax = number, args in rdi, rsi, rdx, r10, r8, r9
#   return value in rax; rcx and r11 are clobbered by the CPU, EVERYTHING else is preserved
syscall_entry:
    movq %rsp, saved_user_rsp(%rip)          # IF is masked by SFMASK, so this global is safe
    movq syscall_kernel_rsp(%rip), %rsp      # current task's kernel stack

    pushq saved_user_rsp(%rip)
    pushq %rcx                               # user rip
    pushq %r11                               # user rflags
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %r8
    pushq %r9
    pushq %r10
    subq $8, %rsp                            # 9 pushes = 72 bytes; pad so rsp is 16-aligned at the call

    cld

    # Shift syscall regs into SysV C args:
    # syscall_handler(num, a1, a2, a3, a4, a5)
    movq %r8,  %r9               # a5
    movq %r10, %r8               # a4  (r10 replaces rcx in the syscall ABI)
    movq %rdx, %rcx              # a3
    movq %rsi, %rdx              # a2
    movq %rdi, %rsi              # a1
    movq %rax, %rdi              # num

    call syscall_handler         # result comes back in rax

    addq $8, %rsp
    popq %r10
    popq %r9
    popq %r8
    popq %rdx
    popq %rsi
    popq %rdi
    popq %r11
    popq %rcx
    popq %rsp                    # restore user stack last
    sysretq
