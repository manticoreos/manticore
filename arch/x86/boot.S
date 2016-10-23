.section .boot
.align 16
init_stack_bottom = .
. = . + 4096
init_stack_top = .

.text

.code32

.globl start
start:
	cli
	mov	$init_stack_top, %esp
	call	start_kernel
	hlt
