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

.align 16
.globl raw_strncpy_from_user
.type raw_strncpy_from_user, @function
raw_strncpy_from_user:
	/*
	 * %rdi: dest
	 * %rsi: src
	 * %rdx: len
	 */
	push	%rdi
	movq	$raw_copy_from_user_fixup, %rdi
	callq	page_fault_set_fixup
	pop	%rdi

	dec	%rdx	/* leave room for NULL-terminator */
1:
	movb	(%rsi), %al
	cmpb	$0x0, %al
	je	2f

	movb	%al, (%rdi)

	inc	%rdi
	inc	%rsi

	dec	%rdx
	cmpq	$0x0, %rdx
	je	2f

	jmp	1b

2:
	movb	$0x0, (%rdi)

	xorq	%rdi, %rdi
	callq	page_fault_set_fixup
	xorq	%rax, %rax
	retq
