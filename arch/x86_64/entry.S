#include <arch/interrupt-defs.h>
#include <arch/processor.h>
#include <arch/segment.h>

.macro EXCEPTION_ENTRY name, has_error_code
.globl \name
.type \name, @function
\name:
	.if \has_error_code == 0
	pushq	$0
	.endif
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rbp
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%rax

	mov	%rsp, %rdi
	subq	$8, %rsp	# Keep %rsp 16-byte aligned

	call	do_\name

	addq	$8, %rsp
	popq	%rax
	popq	%rbx
	popq	%rcx
	popq	%rdx
	popq	%rbp
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	add	$8, %rsp

	iretq
.endm

.macro EXCEPTION_NOERROR_ENTRY name
	EXCEPTION_ENTRY \name, 0
.endm

.macro EXCEPTION_ERROR_ENTRY name
	EXCEPTION_ENTRY \name, 1
.endm

.text

EXCEPTION_NOERROR_ENTRY	x86_divide_error_exception
EXCEPTION_NOERROR_ENTRY	x86_debug_exception
EXCEPTION_NOERROR_ENTRY	x86_nmi_interrupt
EXCEPTION_NOERROR_ENTRY	x86_breakpoint_exception
EXCEPTION_NOERROR_ENTRY	x86_overflow_exception
EXCEPTION_NOERROR_ENTRY	x86_bound_range_exceeded_exception
EXCEPTION_NOERROR_ENTRY	x86_invalid_opcode_exception
EXCEPTION_NOERROR_ENTRY	x86_device_not_available_exception
EXCEPTION_ERROR_ENTRY	x86_double_fault_exception
EXCEPTION_ERROR_ENTRY	x86_invalid_tss_exception
EXCEPTION_ERROR_ENTRY	x86_segment_not_present
EXCEPTION_ERROR_ENTRY	x86_stack_fault_exception
EXCEPTION_ERROR_ENTRY	x86_general_protection_exception
EXCEPTION_ERROR_ENTRY	x86_page_fault_exception
EXCEPTION_NOERROR_ENTRY	x86_x87_fpu_floating_point_error
EXCEPTION_ERROR_ENTRY	x86_alignment_check_exception
EXCEPTION_NOERROR_ENTRY	x86_machine_check_exception
EXCEPTION_NOERROR_ENTRY	x86_simd_floating_point_exception
EXCEPTION_NOERROR_ENTRY	x86_virtualization_exception

/* An array of interrupt entries, which pass interrupt vector as the error code
 * to the interrupt handler. */
.align 16
.global interrupt_entries
interrupt_entries:
vector = NR_EXCEPTIONS
.rept NR_INTERRUPTS
    .align 16
    pushq $vector
    jmp interrupt
    vector = vector + 1
.endr

EXCEPTION_ERROR_ENTRY	interrupt

/*
 * Entry point for the syscall instruction.
 *
 * User space executes the syscall instruction to call into a system call. The
 * instruction traps and starts executing here.
 *
 * The calling conventions are the same as in Linux:
 *
 *	rax	system call number
 *	rdi	arg1
 *	rsi	arg2
 *	rdx	arg3
 *	r10	arg4
 *	r8	arg5
 *	r9	arg6
 */
.align 16
.globl syscall_entry
.type syscall_entry, @function
syscall_entry:
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rbp
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx

	pushq	%r9
	movq	%r8,  %r9
	movq	%rcx, %r8
	movq	%rdx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %rsi
	movq	%rax, %rdi
	callq	syscall
	add	$8, %rsp

	popq	%rbx
	popq	%rcx
	popq	%rdx
	popq	%rbp
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15

	sysretq

.align 16
.globl ret_to_userspace
.type ret_to_userspace, @function
ret_to_userspace:
	movq	$X86_USER_DS, %rax
	mov	%ax, %ds
	mov	%ax, %es

	push	%rax			# %ss
	push	%rsi			# %rsp
	pushfq				# %rflags
	orq	$X86_RFLAGS_IF, (%rsp)	# %rflags |= IF
	pushq	$X86_USER_CS		# %cs
	push	%rdi			# %rip

	iretq
