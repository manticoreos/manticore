#ifndef X86_PROCESSOR_H
#define X86_PROCESSOR_H

#include <kernel/const.h>

/*
 * RFLAGS:
 */
#define X86_RFLAGS_IF		_UL_BIT(9)

/*
 * CR0:
 */
#define X86_CR0_PE		_UL_BIT(0)
#define X86_CR0_PG		_UL_BIT(31)

/*
 * CR4:
 */
#define X86_CR4_PAE		_UL_BIT(5)

#endif
