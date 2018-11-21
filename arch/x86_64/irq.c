#include <kernel/irq.h>

#include <arch/interrupt-defs.h>

#include <kernel/errno.h>
#include <kernel/printf.h>

#include <stddef.h>

struct irq_service {
	irq_handler_t	handler;
	void		*arg;
};

static unsigned int next_vector = NR_EXCEPTIONS;
static struct irq_service irq_services[NR_INTERRUPTS];

irq_vector_t request_irq(irq_handler_t handler, void *arg)
{
	irq_vector_t vector = next_vector++;
	if (vector < NR_EXCEPTIONS || vector > NR_INTERRUPT_VECTORS) {
		return -EINVAL;
	}
	size_t idx = vector - NR_EXCEPTIONS;
	struct irq_service *service = &irq_services[idx];
	if (service->handler) {
		return -EINVAL;
	}
	service->handler = handler;
	service->arg = arg;
	return vector;
}

void handle_interrupt(irq_vector_t vector)
{
	if (vector < NR_EXCEPTIONS || vector > NR_INTERRUPT_VECTORS) {
		return;
	}
	size_t idx = vector - NR_EXCEPTIONS;
	struct irq_service *service = &irq_services[idx];
	if (service->handler) {
		service->handler(service->arg);
	} else {
		printf("warning: unhandled interrupt %d\n", vector);
	}
	end_of_interrupt();
}
