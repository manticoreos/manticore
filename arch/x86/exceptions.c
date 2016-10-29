#include <kernel/interrupts.h>

#include <kernel/panic.h>
#include <arch/segment.h>

#include <stddef.h>
#include <stdint.h>

#define X86_INTERRUPT_DE	0	/* Divide Error Exception */
#define X86_INTERRUPT_DB	1	/* Debug Exception*/
#define X86_INTERRUPT_NMI	2	/* NMI Interrupt */
#define X86_INTERRUPT_BP	3	/* Breakpoint Exception */
#define X86_INTERRUPT_OF	4	/* Overflow Exception */
#define X86_INTERRUPT_BR	5	/* BOUND Range Exceeded Exception */
#define X86_INTERRUPT_UD	6	/* Invalid Opcode Exception */
#define X86_INTERRUPT_NM	7	/* Device Not Available Exception */
#define X86_INTERRUPT_DF	8	/* Double Fault Exception */
#define X86_INTERRUPT_TS	10	/* Invalid TSS Exception */
#define X86_INTERRUPT_NP	11	/* Segment Not Present */
#define X86_INTERRUPT_SS	12	/* Stack Fault Exception */
#define X86_INTERRUPT_GP	13	/* General Protection Exception */
#define X86_INTERRUPT_PF	14	/* Page-Fault Exception */
#define X86_INTERRUPT_MF	16	/* x87 FPU Floating-Point Error */
#define X86_INTERRUPT_AC	17	/* Alignment Check Exception */
#define X86_INTERRUPT_MC	18	/* Machine-Check Exception */
#define X86_INTERRUPT_XM	19	/* SIMD Floating-Point Exception */
#define X86_INTERRUPT_VE	20	/* Virtualization Exception */

struct idt_entry {
	uint16_t	offset_1;	// Offset 15..0
	uint16_t	selector;	// Segment selector
	unsigned	ist  : 3;	// Interrupt stack table
	unsigned	zero : 5;	// Zeros
	unsigned	type : 5;	// Type
	unsigned	dpl  : 2;	// Descriptor privilege level
	unsigned	p    : 1;	// Segment present flag
	uint16_t	offset_2;	// Offset 31..16
	uint32_t	offset_3;	// Offset 63..32
	uint32_t	reserved;	// Reserved
};

#define IDT_ENTRY_OFFSET_1(x) ((x) & 0xffff)
#define IDT_ENTRY_OFFSET_2(x) (((x) >> 16) & 0xffff)
#define IDT_ENTRY_OFFSET_3(x) ((x) >> 32)

#define X86_INTERRUPT_GATE	14
#define X86_TRAP_GATE		15
#define X86_NR_INTERRUPTS	256

void init_idt_entry(struct idt_entry *id, uint8_t type, void *offset)
{
	id->offset_1 = IDT_ENTRY_OFFSET_1((uint64_t) offset);
	id->offset_2 = IDT_ENTRY_OFFSET_2((uint64_t) offset);
	id->offset_3 = IDT_ENTRY_OFFSET_3((uint64_t) offset);
	id->selector = X86_KERNEL_CS;
	id->zero = 0;
	id->type = type;
	id->dpl = 0;
	id->p = 1;
	id->reserved = 0;
}

struct idt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static void load_idt(struct idt_ptr *idtp)
{
        asm volatile (
                "lidt %0"
                :
                :"m" (*idtp)
                );
}

extern struct idt_entry *boot_idt; /* boot.S */
struct idt_ptr idtp;

struct exception_frame {
	uint64_t	regs[15];
	uint64_t	error_code;
	uint64_t	rip;
	uint64_t	cs;
	uint64_t	rflags;
	uint64_t	rsp;
	uint64_t	ss;
};

extern void x86_divide_error_exception(void);
extern void x86_debug_exception(void);
extern void x86_nmi_interrupt(void);
extern void x86_breakpoint_exception(void);
extern void x86_overflow_exception(void);
extern void x86_bound_range_exceeded_exception(void);
extern void x86_invalid_opcode_exception(void);
extern void x86_device_not_available_exception(void);
extern void x86_double_fault_exception(void);
extern void x86_invalid_tss_exception(void);
extern void x86_segment_not_present(void);
extern void x86_stack_fault_exception(void);
extern void x86_general_protection_exception(void);
extern void x86_page_fault_exception(void);
extern void x86_x87_fpu_floating_point_error(void);
extern void x86_alignment_check_exception(void);
extern void x86_machine_check_exception(void);
extern void x86_simd_floating_point_exception(void);
extern void x86_virtualization_exception(void);

void do_x86_divide_error_exception(struct exception_frame *ef)
{
	panic("Divide error exception");
}

void do_x86_debug_exception(struct exception_frame *ef)
{
	panic("Debug exception");
}

void do_x86_nmi_interrupt(struct exception_frame *ef)
{
	panic("NMI interrupt");
}

void do_x86_breakpoint_exception(struct exception_frame *ef)
{
	panic("Breakpoint exception");
}

void do_x86_overflow_exception(struct exception_frame *ef)
{
	panic("Overflow exception");
}

void do_x86_bound_range_exceeded_exception(struct exception_frame *ef)
{
	panic("BOUND range exceeded exception");
}

void do_x86_invalid_opcode_exception(struct exception_frame *ef)
{
	panic("Invalid opcode exception");
}

void do_x86_device_not_available_exception(struct exception_frame *ef)
{
	panic("Device not available exception");
}

void do_x86_double_fault_exception(struct exception_frame *ef)
{
	panic("Double fault exception");
}

void do_x86_invalid_tss_exception(struct exception_frame *ef)
{
	panic("Invalid TSS exception");
}

void do_x86_segment_not_present(struct exception_frame *ef)
{
	panic("Segment not present");
}

void do_x86_stack_fault_exception(struct exception_frame *ef)
{
	panic("Stack fault exception");
}

void do_x86_general_protection_exception(struct exception_frame *ef)
{
	panic("General protection exception");
}

void do_x86_page_fault_exception(struct exception_frame *ef)
{
	panic("Page fault exception");
}

void do_x86_x87_fpu_floating_point_error(struct exception_frame *ef)
{
	panic("x87 FPU floating-point error");
}

void do_x86_alignment_check_exception(struct exception_frame *ef)
{
	panic("Alignment check exception");
}

void do_x86_machine_check_exception(struct exception_frame *ef)
{
	panic("Machine-check exception");
}

void do_x86_simd_floating_point_exception(struct exception_frame *ef)
{
	panic("SIMD floating-point exception");
}

void do_x86_virtualization_exception(struct exception_frame *ef)
{
	panic("Virtualization exception");
}

void arch_init_interrupts(void)
{
	init_idt_entry(boot_idt + X86_INTERRUPT_DE, X86_TRAP_GATE, x86_divide_error_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_DB, X86_TRAP_GATE, x86_debug_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_NMI, X86_INTERRUPT_GATE, x86_nmi_interrupt);
	init_idt_entry(boot_idt + X86_INTERRUPT_BP, X86_TRAP_GATE, x86_breakpoint_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_OF, X86_TRAP_GATE, x86_overflow_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_BR, X86_TRAP_GATE, x86_bound_range_exceeded_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_UD, X86_TRAP_GATE, x86_invalid_opcode_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_NM, X86_TRAP_GATE, x86_device_not_available_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_DF, X86_TRAP_GATE, x86_double_fault_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_TS, X86_TRAP_GATE, x86_invalid_tss_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_NP, X86_TRAP_GATE, x86_segment_not_present);
	init_idt_entry(boot_idt + X86_INTERRUPT_SS, X86_TRAP_GATE, x86_stack_fault_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_GP, X86_TRAP_GATE, x86_general_protection_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_PF, X86_TRAP_GATE, x86_page_fault_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_MF, X86_TRAP_GATE, x86_x87_fpu_floating_point_error);
	init_idt_entry(boot_idt + X86_INTERRUPT_AC, X86_TRAP_GATE, x86_alignment_check_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_MC, X86_TRAP_GATE, x86_machine_check_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_XM, X86_TRAP_GATE, x86_simd_floating_point_exception);
	init_idt_entry(boot_idt + X86_INTERRUPT_VE, X86_TRAP_GATE, x86_virtualization_exception);

	idtp.limit = (sizeof(struct idt_entry) * X86_NR_INTERRUPTS) - 1;
	idtp.base = (uint64_t) boot_idt;
	load_idt(&idtp);
}
