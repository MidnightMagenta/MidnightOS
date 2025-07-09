bits 64

; table of extern and global symbols to import/export
extern kernel_entry
extern kernel_main
extern _init
extern _fini
global _start

section .bss.boot_stack
	align 16
	_stack_bottom:				; kernel's bootstrap stack
		resb 0x4000
	_stack_top:

section .text
	_start:
		cli						; mask all interupts
		mov rsp, _stack_top		; set stack pointer to the kernel's stack
		xor rbp, rbp			; clear rbp to create sentinel stack frame
		push rbp				; set the sentinel stack frame
		mov rbp, rsp
		call kernel_entry
		call _init				; call crt _init function
		call kernel_main		; call kernel_main. At this point global objects can be used
		call _fini
		cli
	_halt:						; halt the processor, this statement should never be reached
		hlt
		jmp _halt
