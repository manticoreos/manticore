#include <kernel/errno.h>

.align 16
raw_copy_from_user_fixup:
	xorq	%rdi, %rdi
	callq	page_fault_set_fixup

	movq	$-EFAULT, %rax
	retq

.align 16
.globl raw_copy_from_user
.type raw_copy_from_user, @function
raw_copy_from_user:
	push	%rdi
	movq	$raw_copy_from_user_fixup, %rdi
	callq	page_fault_set_fixup
	pop	%rdi

	movq	%rdx, %rcx
	rep movsb

	xorq	%rdi, %rdi
	callq	page_fault_set_fixup

	xorq	%rax, %rax
	retq
