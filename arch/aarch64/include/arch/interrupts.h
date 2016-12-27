#ifndef ARM64_INTERRUPTS_H
#define ARM64_INTERRUPTS_H

#include <arch/processor.h>

#include <stdbool.h>

/// Disables local interrupts.
static inline void arch_local_interrupt_disable(void)
{
	asm volatile(
		"msr	daifset, #2"
		:
		:
		: "memory");
}

/// Enables local interrupts.
static inline void arch_local_interrupt_enable(void)
{
	asm volatile(
		"msr	daifclr, #2"
		:
		:
		: "memory");
}

/// Returns the current local interrupt mask.
///
/// \return the current local interrupt mask.
static inline unsigned long arch_local_interrupt_mask(void)
{
	unsigned long flags;

	asm volatile(
		"mrs	%0, daif"
		: "=r" (flags)
		:
		: "memory");
	return flags;
}

/// Disables local interrupts and returns prior interrupt mask.
///
/// \return the prior local interrupt mask (before interrupts were disabled).
static inline unsigned long arch_local_interrupt_save(void)
{
	unsigned long flags = arch_local_interrupt_mask();
	arch_local_interrupt_disable();
	return flags;
}

/// Restores local interrupt mask from \flags.
///
/// This function either enables or disabled local interrupts depending on the
/// interrupt mask specified in \flags.
///
/// \param flags Interrupt mask.
static inline void arch_local_interrupt_restore(unsigned long flags)
{
	asm volatile(
		"msr	daif, %0\n"
		:
		: "r" (flags)
		: "memory", "cc");
}

/// Returns `true` if local interrupts are disabled; otherwise returns `false`.
///
/// \param flags The interrupt mask.
/// \return `true` if local interrupts are disabled; otherwise returns `false`.
static inline bool arch_local_interrupt_is_disabled(unsigned long flags)
{
	return flags & ARM64_SPSR_I;
}

#endif
