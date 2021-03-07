#ifndef KERNEL_CONST_H
#define KERNEL_CONST_H

#ifdef __ASSEMBLY__
#define _AC(x,y) x
#else
#define _AC(x,y) (x##y)
#endif

#define _UL(x) _AC(x, UL)
#define _ULL(x) _AC(x, ULL)

#define _UL_BIT(x) (_UL(1) << (x))
#define _ULL_BIT(x) (_ULL(1) << (x))

#endif
