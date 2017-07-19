#include <arch/syscall.h>

#include <arch/cpu.h>
#include <arch/msr.h>
#include <arch/segment.h>

#include <stdint.h>

#define IA_32_STAR_SYSRET_SHIFT 48
#define IA_32_STAR_SYSCALL_SHIFT 32

void init_syscall(void)
{
	uint64_t star = ((uint64_t)X86_KERNEL_DS << IA_32_STAR_SYSRET_SHIFT) |
			((uint64_t)X86_KERNEL_CS << IA_32_STAR_SYSCALL_SHIFT);
	wrmsr(X86_IA32_STAR, star);
	wrmsr(X86_IA32_LSTAR, (uint64_t)syscall_entry);
	wrmsr(X86_IA32_FMASK, 0);
	wrmsr(X86_IA32_EFER, rdmsr(X86_IA32_EFER) | X86_IA32_EFER_SCE);
}
