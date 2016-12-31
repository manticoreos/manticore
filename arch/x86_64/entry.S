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