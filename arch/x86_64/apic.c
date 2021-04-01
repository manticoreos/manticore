/*
 * Advanced programmable interrupt controller (APIC) driver for x86
 *
 * The APIC is local to a CPU, and is responsible for interrupt handling,
 * timers, and inter-process interrupts (IPIs).
 */
#include <arch/apic.h>

#include <arch/cpu.h>
#include <arch/cpuid.h>
#include <arch/msr.h>

#include <kernel/printf.h>
#include <kernel/panic.h>
#include <kernel/irq.h>

#include <stdbool.h>

/* Local APIC registers in MSR offsets. Specified in Table 10-6 ("Local APIC
   Register Address Map Supported by x2APIC") of Intel SDM.  */
enum {
	APIC_EOI = 0x80b,
	APIC_SPIV = 0x80f,
	APIC_LVT_TIMER  = 0x832,
	ACPI_LVT_LINT0  = 0x835,
	APIC_TIMER_IC = 0x838,		// Timer initial count (IC) register
	APIC_TIMER_CC = 0x839,		// Timer current count (DC) register
	APIC_TIMER_DC  = 0x83e,		// Timer divide configuration (DD) register
};

enum {
	APIC_LVT_TIMER_PERIODIC = 0x01 << 17,
};

/* MSI message data register flags. Specified in Section 10.11.2
   ("Message Data Register Format)") of Intel SDM. */
enum {
	/* Delivery mode: */
	__MSI_DELIVERY_SHIFT = 8,
	MSI_DELIVERY_FIXED = 0b000UL << __MSI_DELIVERY_SHIFT,

	/* Trigger mode: */
	__MSI_TRIGGER_SHIFT = 15,
	MSI_TRIGGER_EDGE = 1UL << __MSI_TRIGGER_SHIFT,
};

static uint64_t _apic_base;

void apic_compose_msi_msg(struct msi_message *msg, uint8_t vector, uint8_t dest_id)
{
	msg->msg_addr = (_apic_base & 0xfff00000ULL) | (dest_id << 12);
	msg->msg_data = MSI_TRIGGER_EDGE | MSI_DELIVERY_FIXED | vector;
}

static void apic_write(uint32_t addr, uint64_t val)
{
	wrmsr(addr, val);
}

static void apic_timer_intr(void *arg)
{
	/* An interrupt wakes up the kernel from its idle loop that blocks on
	   an arch_halt_cpu() call.  */
}

static void apic_eoi(void)
{
	apic_write(APIC_EOI, 0);
}

void end_of_interrupt(void)
{
	apic_eoi();
}

static void apic_timer_init(void)
{
	irq_vector_t vector = request_irq(apic_timer_intr, NULL);
	if (vector < 0) {
		panic("Unable to initialize APIC timer.");
	}
	apic_write(APIC_LVT_TIMER, vector | APIC_LVT_TIMER_PERIODIC);
	apic_write(APIC_TIMER_DC, 0x3);
	/* TODO: Configure timer counter using unit of time. */
	apic_write(APIC_TIMER_IC, 100000000);
}

static inline bool probe_x2apic(void)
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	cpuid(0x01, &eax, &ebx, &ecx, &edx);
	return ecx & X86_CPUID_FEATURE_ECX_X2APIC;
}

bool apic_is_bsp(void)
{
	return (rdmsr(X86_IA32_APIC_BASE) & X86_IA32_APIC_BASE_BSP) == X86_IA32_APIC_BASE_BSP;
}

void init_apic(void)
{
	if (!probe_x2apic()) {
		printf("No x2APIC found\n");
		return;
	}
	_apic_base = rdmsr(X86_IA32_APIC_BASE);
	wrmsr(X86_IA32_APIC_BASE, _apic_base | X86_IA32_APIC_BASE_EXTD | X86_IA32_APIC_BASE_EN);

	_apic_base &= ~X86_IA32_APIC_BASE_BSP;
	printf("Found x2APIC at %lx\n", _apic_base);

	apic_write(ACPI_LVT_LINT0, 0);
	apic_write(APIC_SPIV, 0x1ff);

	apic_timer_init();
}
