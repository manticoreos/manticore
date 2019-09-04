#include <kernel/cpu.h>


/// The arch_cpu_halt function halts the CPU and makes it wait for an
/// interrupt to wake it up. The HLT instruction is a privileged instruction.
/// When a HLT instruction is executed in an IA-32 processor only the
/// logical processor in the physcial processor remain active, unless they
/// are each individually halted by executing a HLT instruction.

void arch_halt_cpu(void)
{
	asm volatile (
		"wfi"
		:
		:
		: "memory");
}
