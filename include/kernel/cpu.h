#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

/// Halt the current CPU, and wait for an interrupt to wake it up.
void arch_halt_cpu(void);

#endif
