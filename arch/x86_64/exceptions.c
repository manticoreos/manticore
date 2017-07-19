#include <arch/exceptions.h>

#include <kernel/printf.h>
#include <kernel/panic.h>

#include <arch/segment.h>
#include <arch/cpu.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

void init_idt_entry(struct idt_entry *id, uint8_t type, unsigned ist, void *offset)
{
	id->offset_1 = IDT_ENTRY_OFFSET_1((uint64_t) offset);
	id->offset_2 = IDT_ENTRY_OFFSET_2((uint64_t) offset);
	id->offset_3 = IDT_ENTRY_OFFSET_3((uint64_t) offset);
	id->selector = X86_KERNEL_CS;
	id->ist = ist;
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

static struct idt_entry idt[256];
static struct idt_ptr idtp;

struct exception_frame {
	uint64_t	regs[15];
	uint64_t	error_code;
	uint64_t	rip;
	uint64_t	cs;
	uint64_t	rflags;
	uint64_t	rsp;
	uint64_t	ss;
};

enum {
	REG_RAX	= 0,
	REG_RBX = 1,
	REG_RCX = 2,
	REG_RDX = 3,
	REG_RBP = 4,
	REG_RSI = 5,
	REG_RDI	= 6,
	REG_R8	= 7,
	REG_R9	= 8,
	REG_R10	= 9,
	REG_R11	= 10,
	REG_R12	= 11,
	REG_R13	= 12,
	REG_R14 = 13,
	REG_R15	= 14,
};

static void dump_exception_frame(struct exception_frame *ef)
{
	printf("Registers:\n");
	printf("  RIP=%016lx CS=%016lx RFLAGS=%016lx RSP=%016lx SS=%016lx\n",
		ef->rip, ef->cs, ef->rflags, ef->rsp, ef->ss);
	printf("  RAX=%016lx RBX=%016lx RCX=%016lx RDX=%016lx RBP=%016lx\n",
		ef->regs[REG_RAX], ef->regs[REG_RBX], ef->regs[REG_RCX], ef->regs[REG_RDX], ef->regs[REG_RBP]);
	printf("  RSI=%016lx RDI=%016lx R8=%016lx  R9=%016lx  R10=%016lx\n",
		ef->regs[REG_RSI], ef->regs[REG_RDI], ef->regs[REG_R8], ef->regs[REG_R9], ef->regs[REG_R10]);
	printf("  R11=%016lx R12=%016lx R13=%016lx R14=%016lx R15=%016lx\n",
		ef->regs[REG_R11], ef->regs[REG_R12], ef->regs[REG_R13], ef->regs[REG_R14], ef->regs[REG_R15]);
}

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
	printf("Divide error exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_debug_exception(struct exception_frame *ef)
{
	printf("Debug exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_nmi_interrupt(struct exception_frame *ef)
{
	printf("NMI interrupt\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_breakpoint_exception(struct exception_frame *ef)
{
	printf("Breakpoint exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_overflow_exception(struct exception_frame *ef)
{
	printf("Overflow exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_bound_range_exceeded_exception(struct exception_frame *ef)
{
	printf("BOUND range exceeded exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_invalid_opcode_exception(struct exception_frame *ef)
{
	printf("Invalid opcode exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_device_not_available_exception(struct exception_frame *ef)
{
	printf("Device not available exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_double_fault_exception(struct exception_frame *ef)
{
	printf("Double fault exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_invalid_tss_exception(struct exception_frame *ef)
{
	printf("Invalid TSS exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_segment_not_present(struct exception_frame *ef)
{
	printf("Segment not present\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_stack_fault_exception(struct exception_frame *ef)
{
	printf("Stack fault exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_general_protection_exception(struct exception_frame *ef)
{
	printf("General protection exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_page_fault_exception(struct exception_frame *ef)
{
	uint64_t addr = x86_read_cr2();
	printf("Page fault at address %016lx\n", addr);
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_x87_fpu_floating_point_error(struct exception_frame *ef)
{
	printf("x87 FPU floating-point error\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_alignment_check_exception(struct exception_frame *ef)
{
	printf("Alignment check exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_machine_check_exception(struct exception_frame *ef)
{
	printf("Machine-check exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_simd_floating_point_exception(struct exception_frame *ef)
{
	printf("SIMD floating-point exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void do_x86_virtualization_exception(struct exception_frame *ef)
{
	printf("Virtualization exception\n");
	dump_exception_frame(ef);
	panic("Halted");
}

void init_idt(void)
{
	memset(idt, 0, sizeof(idt));
	init_idt_entry(idt + X86_INTERRUPT_DE, X86_TRAP_GATE, 0, x86_divide_error_exception);
	init_idt_entry(idt + X86_INTERRUPT_DB, X86_TRAP_GATE, 1, x86_debug_exception);
	init_idt_entry(idt + X86_INTERRUPT_NMI, X86_INTERRUPT_GATE, 1, x86_nmi_interrupt);
	init_idt_entry(idt + X86_INTERRUPT_BP, X86_TRAP_GATE, 0, x86_breakpoint_exception);
	init_idt_entry(idt + X86_INTERRUPT_OF, X86_TRAP_GATE, 0, x86_overflow_exception);
	init_idt_entry(idt + X86_INTERRUPT_BR, X86_TRAP_GATE, 0, x86_bound_range_exceeded_exception);
	init_idt_entry(idt + X86_INTERRUPT_UD, X86_TRAP_GATE, 0, x86_invalid_opcode_exception);
	init_idt_entry(idt + X86_INTERRUPT_NM, X86_TRAP_GATE, 0, x86_device_not_available_exception);
	init_idt_entry(idt + X86_INTERRUPT_DF, X86_TRAP_GATE, 1, x86_double_fault_exception);
	init_idt_entry(idt + X86_INTERRUPT_TS, X86_TRAP_GATE, 0, x86_invalid_tss_exception);
	init_idt_entry(idt + X86_INTERRUPT_NP, X86_TRAP_GATE, 0, x86_segment_not_present);
	init_idt_entry(idt + X86_INTERRUPT_SS, X86_TRAP_GATE, 0, x86_stack_fault_exception);
	init_idt_entry(idt + X86_INTERRUPT_GP, X86_TRAP_GATE, 0, x86_general_protection_exception);
	init_idt_entry(idt + X86_INTERRUPT_PF, X86_TRAP_GATE, 0, x86_page_fault_exception);
	init_idt_entry(idt + X86_INTERRUPT_MF, X86_TRAP_GATE, 0, x86_x87_fpu_floating_point_error);
	init_idt_entry(idt + X86_INTERRUPT_AC, X86_TRAP_GATE, 0, x86_alignment_check_exception);
	init_idt_entry(idt + X86_INTERRUPT_MC, X86_TRAP_GATE, 1, x86_machine_check_exception);
	init_idt_entry(idt + X86_INTERRUPT_XM, X86_TRAP_GATE, 0, x86_simd_floating_point_exception);
	init_idt_entry(idt + X86_INTERRUPT_VE, X86_TRAP_GATE, 0, x86_virtualization_exception);

	idtp.limit = (sizeof(struct idt_entry) * X86_NR_INTERRUPTS) - 1;
	idtp.base = (uint64_t) idt;
	load_idt(&idtp);
}
