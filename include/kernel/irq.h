#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

typedef int irq_vector_t;
typedef void (*irq_handler_t)(void *);

irq_vector_t request_irq(irq_handler_t, void *arg);

void handle_interrupt(irq_vector_t vector);
void end_of_interrupt(void);

#endif
