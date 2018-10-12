#include <arch/apic.h>

#include <arch/cpu.h>
#include <arch/cpuid.h>
#include <arch/msr.h>

#include <kernel/printf.h>

#include <stdbool.h>

/* Local APIC registers in MSR offsets. Specified in Table 10-6 ("Local APIC
   Register Address Map Supported by x2APIC") of Intel SDM.  */
enum {
	APIC_EOI = 0x80b,
	APIC_SPIV = 0x80f,
	APIC_LVT_TIMER  = 0x832,
	ACPI_LVT_LINT0  = 0x835,
	APIC_TIMER_IC = 0x838,
	APIC_TIMER_CC = 0x839,
	APIC_TIMER_DC  = 0x83e,
};

static uint64_t _apic_base;

static void apic_write(uint32_t addr, uint64_t val)
{
	wrmsr(addr, val);
}

static void apic_eoi(void)
{
	apic_write(APIC_EOI, 0);
}

void end_of_interrupt(void)
{
	apic_eoi();
}

static inline bool probe_x2apic(void)
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuid(0x01, &eax, &ebx, &ecx, &edx);
	return ecx & X86_CPUID_FEATURE_ECX_X2APIC;
}

void init_apic(void)
{
	if (!probe_x2apic()) {
		printf("No x2APIC found\n");
		return;
	}
	_apic_base = rdmsr(X86_IA32_APIC_BASE);
	printf("Found x2APIC at %lx\n", _apic_base);

	wrmsr(X86_IA32_APIC_BASE, _apic_base | X86_IA32_APIC_BASE_EXTD | X86_IA32_APIC_BASE_EN);
	apic_write(ACPI_LVT_LINT0, 0);
	apic_write(APIC_SPIV, 0x1ff);
}
