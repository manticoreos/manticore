#include <kernel/panic.h>

#include <kernel/console.h>
#include <kernel/printf.h>
#include <kernel/cpu.h>

#include <arch/interrupts.h>

//////////////////////////////////////////////////////////////////////////////
// \breif Implementation of panic function, times when interrupts are disabled
//        panics will be called with appropriate message
//
// \param panic message
// \returns none
// //////////////////////////////////////////////////////////////////////////
void panic(char *msg)
{
	console_write("Kernel panic: ");
	console_write(msg);
	console_write("\n");
	arch_local_interrupt_disable();
	for (;;) {
		arch_halt_cpu();
	}
}

void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function)
{
	static char msg[64];
	sprintf(msg, "%s:%d: %s: Assertion %s failed", file, line, function, assertion);
	panic(msg);
}
