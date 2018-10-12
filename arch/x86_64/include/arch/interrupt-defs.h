#ifndef ARCH_INTERRUPT_DEFS_H
#define ARCH_INTERRUPT_DEFS_H

/* Number of interrupt vectors, or interrupt entry points: */
#define NR_INTERRUPT_VECTORS	256
/* Number of exceptions, or processor-generated interrupts: */
#define NR_EXCEPTIONS		32
/* Number of hardware and software interrupts: */
#define NR_INTERRUPTS		(NR_INTERRUPT_VECTORS - NR_EXCEPTIONS)

#endif
