#include <kernel/irq.h>

#include <arch/interrupt-defs.h>

#include <kernel/errno.h>
#include <kernel/printf.h>

#include <stddef.h>

static unsigned int next_vector = NR_EXCEPTIONS;
static irq_handler_t irq_handlers[NR_INTERRUPTS];

irq_vector_t request_irq(irq_handler_t handler)
{
	irq_vector_t vector = next_vector++;
	if (vector < NR_EXCEPTIONS || vector > NR_INTERRUPT_VECTORS) {
		return -EINVAL;
	}
	size_t idx = vector - NR_EXCEPTIONS;
	if (irq_handlers[idx]) {
		return -EINVAL;
	}
	irq_handlers[idx] = handler;
	return vector;
}

void handle_interrupt(irq_vector_t vector)
{
	if (vector < NR_EXCEPTIONS || vector > NR_INTERRUPT_VECTORS) {
		return;
	}
	size_t idx = vector - NR_EXCEPTIONS;
	irq_handler_t handler = irq_handlers[idx];
	if (handler) {
		handler();
	} else {
		printf("warning: unhandled interrupt %d\n", vector);
	}
	end_of_interrupt();
}
