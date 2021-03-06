#include <arch/processor.h>
#include <arch/vmem-defs.h>
#include <arch/segment.h>
#include <arch/msr.h>

.section .boot

.align 8
.global boot_gdt
boot_gdt:
	.quad 0

	/* Kernel code segment: */
	.quad X86_GDT_ENTRY(X86_GDT_TYPE_CODE | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_L, 0, 0xfffff)

	/* Kernel data segment: */
	.quad X86_GDT_ENTRY(X86_GDT_TYPE_DATA | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_DB, 0, 0xfffff)
boot_gdt_end:

boot_gdt_desc32:
	.short boot_gdt_end - boot_gdt - 1
	.long boot_gdt

.align 8
boot_gdt_desc64:
	.short boot_gdt_end - boot_gdt - 1
	.quad boot_gdt + KERNEL_VMA

/*
 * Boot page mapping has two mappings: (1) identity mapping for first 1
 * GB of physical memory for bootstrap and (2) higher-half kernel
 * mapping once the kernel is up and running.
 */
.align 4096
boot_pml4e:
	.quad boot_pdpte + (X86_PE_RW | X86_PE_P)
	.fill 510, 8, 0
	.quad boot_pdpte + (X86_PE_RW | X86_PE_P)
boot_pdpte:
	.quad boot_pde + (X86_PE_RW | X86_PE_P)
	.fill 509, 8, 0
	.quad boot_pde + (X86_PE_RW | X86_PE_P)
	.quad 0
boot_pde:
	pfn = 0
	.rept 512
		.quad (pfn << 21) + (X86_PE_PS | X86_PE_RW | X86_PE_P)
		pfn = pfn + 1
	.endr

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

	/* Setup 32-bit GDT */
	lgdt	boot_gdt_desc32

        /* Disable paging */
	mov	%cr0, %eax
	and	$~X86_CR0_PG, %eax
	mov	%eax, %cr0

	/* Enable PAE */
	mov	%cr4, %eax
	or	$X86_CR4_PAE, %eax
	mov	%eax, %cr4

	/* Enable SSE instructions */
	mov	%cr4, %eax
	or	$X86_CR4_OSFXSR, %eax
	or	$X86_CR4_OSXMMEXCPT, %eax
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
	/* Setup 64-bit GDT */
	lgdt	boot_gdt_desc64

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

.bss

.align 16
init_stack_bottom = .
. = . + 4096
init_stack_top = .
