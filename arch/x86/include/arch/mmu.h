#ifndef ARCH_MMU_H
#define ARCH_MMU_H

#include <kernel/const.h>

#define X86_PE_PRESENT	_UL_BIT(0)	/* Present */
#define X86_PE_RW	_UL_BIT(1)	/* Read/write */
#define X86_PE_PS	_UL_BIT(7)	/* Page size */

#endif
