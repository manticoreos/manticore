#ifndef ARCH_SEGMENT_H
#define ARCH_SEGMENT_H

#define X86_KERNEL_CS		0x08
#define X86_KERNEL_DS		0x10

#define X86_GDT_TYPE_DATA	(2 << 8)
#define X86_GDT_TYPE_CODE	(8 << 8)
#define X86_GDT_S		(1 << 12)
#define X86_GDT_DPL(level)	((level) << 13)
#define X86_GDT_P		(1 << 15)
#define X86_GDT_L		(1 << 21)
#define X86_GDT_DB		(1 << 22)
#define X86_GDT_G		(1 << 23)

#endif
