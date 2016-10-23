#ifndef KERNEL_CONST_H
#define KERNEL_CONST_H

#ifdef __ASSEMBLY__
#define _UL_BIT(x) (1 << (x))
#else
#define _UL_BIT(x) (1UL << (x))
#endif

#endif
