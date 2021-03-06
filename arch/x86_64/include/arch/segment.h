#ifndef ARCH_SEGMENT_H
#define ARCH_SEGMENT_H

#include <kernel/const.h>

#define X86_KERNEL_CS		0x08
#define X86_KERNEL_DS		0x10
#define X86_USER_CS		0x23
#define X86_USER_DS		0x1b
#define X86_GDT_TSS_IDX		5

#define X86_GDT_TYPE_DATA	(_ULL(2) << (32+8))
#define X86_GDT_TYPE_CODE	(_ULL(8) << (32+8))
#define X86_GDT_S		(_ULL(1) << (32+12))
#define X86_GDT_DPL(level)	(_ULL(level) << (32+13))
#define X86_GDT_P		(_ULL(1) << (32+15))
#define X86_GDT_L		(_ULL(1) << (32+21))
#define X86_GDT_DB		(_ULL(1) << (32+22))
#define X86_GDT_G		(_ULL(1) << (32+23))

#define X86_GDT_ENTRY(flags, base, limit)	\
	((((base) & _ULL(0xff000000)) << 32)	\
         | (flags)				\
         | (((limit) & _ULL(0xf0000)) << 32)	\
         | (((base) & _ULL(0x00ffffff)) << 16) 	\
         | (((limit) & _ULL(0x0ffff))))

#endif
