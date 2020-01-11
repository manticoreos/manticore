#include <kernel/errno.h>

/* Converts a page fault exception to EFAULT error.

   This function is invoked when __memcpy_user_safe() page faults due to a
   invalid memory reference.  This can happen, for example, when user space
   passes a memory address that it has no access to.   */
.align 16
__memcpy_user_fixup:
	xorq	%rdi, %rdi
	callq	page_fault_set_fixup

	movq	$-EFAULT, %rax
	retq

/* Copies N bytes of memory from one memory location to another.

   This function is a version of memcpy() which safely copies memory from and
   to user space buffers.  A page fault is translated into EFAULT error.

     %rdi: destination pointer
     %rsi: source pointer
     %rdx: length of memory region to copy  */
.align 16
.globl __memcpy_user_safe
.type __memcpy_user_safe, @function
__memcpy_user_safe:
	push	%rdi
	movq	$__memcpy_user_fixup, %rdi
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
	movq	$__memcpy_user_fixup, %rdi
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
