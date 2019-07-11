#include <kernel/panic.h>

#include <kernel/console.h>
#include <kernel/printf.h>
#include <kernel/cpu.h>

#include <arch/interrupts.h>

static void do_panic(char *msg)
{
	console_write_str("Kernel panic: ");
	console_write_str(msg);
	console_write_str("\n");
	arch_local_interrupt_disable();
	for (;;) {
		arch_halt_cpu();
	}
}

void panic(const char *fmt, ...)
{
	static char msg[64];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, ARRAY_SIZE(msg), fmt, ap);
	va_end(ap);
	do_panic(msg);
}

void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function)
{
	static char msg[64];
	sprintf(msg, "%s:%d: %s: Assertion %s failed", file, line, function, assertion);
	do_panic(msg);
}
