#ifndef ARCH_MSR_H
#define ARCH_MSR_H

#include <kernel/const.h>

#define X86_IA32_EFER		0xc0000080
#define X86_IA32_EFER_SCE	_UL_BIT(0)
#define X86_IA32_EFER_LME	_UL_BIT(8)
#define X86_IA32_STAR		0xc0000081
#define X86_IA32_LSTAR		0xc0000082
#define X86_IA32_FMASK		0xc0000084

#endif
