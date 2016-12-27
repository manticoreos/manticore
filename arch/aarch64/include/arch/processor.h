#ifndef ARM64_PROCESSOR_H
#define ARM64_PROCESSOR_H

#include <kernel/const.h>

/*
 * Saved Program Status Registers (SPSRs):
 */
#define ARM64_SPSR_A		_UL_BIT(8)	/* SError interrupt mask bit */
#define ARM64_SPSR_I		_UL_BIT(7)	/* IRQ mask bit */
#define ARM64_SPSR_F		_UL_BIT(6)	/* FIQ mask bit */

#endif
