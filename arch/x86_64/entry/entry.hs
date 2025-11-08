#ifndef _NYX_ENTRY_SH
#define _NYX_ENTRY_SH

.macro _PUSH_REGS
push %r15
push %r14
push %r13
push %r12
push %r11
push %r10
push %r9
push %r8
push %rbp
push %rdi
push %rsi
push %rdx
push %rcx
push %rbx
push %rax
.endm

.macro _POP_REGS
pop %rax
pop %rbx
pop %rcx
pop %rdx
pop %rsi
pop %rdi
pop %rbp
pop %r8
pop %r9
pop %r10
pop %r11
pop %r12
pop %r13
pop %r14
pop %r15
.endm

#define PUSH_REGS() _PUSH_REGS
#define POP_REGS() _POP_REGS

.macro ENTRY symbol ecode
.global \symbol
.extern do_\symbol
\symbol:
    .if \ecode == 0
    push $0
    .endif
    PUSH_REGS()
    mov %rsp, %rdi
    mov %rsp, %rbx
    and $-16, %rsp
    sub $8, %rsp
    call do_\symbol
    mov %rbx, %rsp
    POP_REGS()
    .if \ecode == 0
    add $8, %rsp
    .endif
    iretq
.endm

#define ENTRY_NECODE(sym) ENTRY sym, 0
#define ENTRY_ECODE(sym) ENTRY sym, 1

#endif