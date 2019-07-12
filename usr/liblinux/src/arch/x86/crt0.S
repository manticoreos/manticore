.text
.globl _start
.type _start, @function
_start:
	xorq	%rbp, %rbp

	call	main

	movq	%rax, %rdi
	call	exit

	hlt
