#include <arch/processor.h>
#include <arch/segment.h>
#include <arch/mmu.h>
#include <arch/msr.h>

.section .boot

.align 8
.global boot_gdt
boot_gdt:
	.quad 0

	/* Kernel code segment: */
	.long 0
	.long X86_GDT_TYPE_CODE | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_L

	/* Kernel data segment: */
	.long 0
	.long X86_GDT_TYPE_DATA | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_DB
boot_gdt_end:

.align 8
boot_gdt_desc:
	.short boot_gdt_end - boot_gdt - 1
	.quad boot_gdt

/*
 * Identity page mapping for first 1 GiB of physical memory.
 */
.align 4096
boot_pml4e:
	.quad boot_pdpte + (X86_PE_RW | X86_PE_P)
	.fill 511, 8, 0
boot_pdpte:
	.quad boot_pde + (X86_PE_RW | X86_PE_P)
	.fill 511, 8, 0
boot_pde:
	pfn = 0
	.rept 512
		.quad (pfn << 21) + (X86_PE_PS | X86_PE_RW | X86_PE_P)
		pfn = pfn + 1
	.endr

.align 16
init_stack_bottom = .
. = . + 4096
init_stack_top = .

.align 16
.globl boot_idt
boot_idt:
	.fill 512, 8, 0

.align 8
.globl boot_data
boot_data:
	.fill 1, 8, 0

.code32

/*
 * This is the boostrap entry point to the kernel. The assumption here
 * is that the kernel was loaded by a bootloader that already put the
 * CPU into 32-bit protected mode.
 */
.align 16
.globl start
start:
	/* Disable interrupts */
	cli

	/* Store Multiboot data pointer */
	mov	%ebx, boot_data

	/* Disable all interrupts on the legacy PIC */
	mov	$0xff, %al
	out	%al, $0xa1
	out	%al, $0x21

	/* Setup GDT */
	lgdt	boot_gdt_desc

        /* Disable paging */
	mov	%cr0, %eax
	and	$~X86_CR0_PG, %eax
	mov	%eax, %cr0

	/* Enable PAE */
	mov	%cr4, %eax
	or	$X86_CR4_PAE, %eax
	mov	%eax, %cr4

	/* Load page map table */
	mov	$boot_pml4e, %eax
	mov	%eax, %cr3

	/* Enable IA-32e mode */
	mov	$X86_IA32_EFER, %ecx
	rdmsr
	or	$X86_IA32_EFER_LME, %eax
	wrmsr

	/* Enable paging to activate IA-32e mode */
	mov	%cr0, %eax
	or	$X86_CR0_PG, %eax
	mov	%eax, %cr0

	/* Jump to 64-bit code segment... */
	ljmp	$X86_KERNEL_CS, $start64

.code64

start64:
	/* Initialize the remaining segment registers */
	mov	$X86_KERNEL_DS, %eax
	mov	%eax, %ds
	mov	%eax, %es
	mov	%eax, %fs
	mov	%eax, %gs
	mov	%eax, %ss

	/* Initialize stack pointer */
	mov	$init_stack_top, %rsp

	/* Call into the kernel */
	call	start_kernel

	/* If we ever return from start_kernel(), disable interrupts and
	   execute `hlt` instruction in an infinite loop. */
	cli
halt:	hlt
	jmp halt
