.global _start 
.global _idt 
.global _gdt
.extern main

.section .text
_start:
  cli
  mov %rdi, %r15
  xor %rbp, %rbp
  mov $_stack_top, %rsp
  push %rbp
  mov %rsp, %rbp
  call setup_gdt
  call setup_idt
  mov %r15, %rdi
  call main
  cli
1:hlt
  jmp 1b

setup_gdt:
  lgdt gdt_desc
  xor %rax, %rax
  mov $0x10, %rax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  pop %rdi
  mov $0x08, %rax
  push %rax
  push %rdi
  retfq

/* 
  Load the IDT with the ignore_int procedure
*/
setup_idt:
  lea ignore_int(%rip), %rax
  mov %ax, idt_entry(%rip)
  shr $16, %rax
  mov %ax, idt_entry+6(%rip)
  shr $16, %rax
  mov %eax, idt_entry+8(%rip)

  mov (idt_entry), %rax
  mov (idt_entry + 8), %rdx
  lea _idt(%rip), %rdi
  mov $256, %rcx
rp_sidt:
  mov %rax, (%rdi)
  mov %rdx, 8(%rdi)
  add $16, %rdi
  dec %rcx
  jnz rp_sidt
  lidt idt_desc
  ret

ignore_int:
  iretq

.section .data
.align 16
idt_entry:
  .2byte 0
  .2byte 0x0008
  .byte  0
  .byte  0x8E
  .2byte 0
  .4byte 0
  .4byte 0

.align 8
gdt_desc:
  .2byte 256 * 8 - 1
  .8byte _gdt

.align 8
idt_desc:
  .2byte 256 * 8 - 1
  .8byte _idt

.align 0x1000
_idt:
  .fill 512, 8, 0

.align 0x1000
_gdt:
  .8byte 0x0000000000000000
  .8byte 0x00af9a000000ffff
  .8byte 0x00af92000000ffff
  .8byte 0x0000000000000000
  .8byte 0x00aff8000000ffff
  .8byte 0x00aff2000000ffff
  .fill 250, 8, 0

.align 0x1000
_stack_bottom:
  .skip 0x4000
_stack_top:
