#ifndef X86_GDT_H
#define X86_GDT_H

#include <stdint.h>

struct gdt_desc64 {
	uint16_t	limit;
	uint64_t	base;
} __attribute__ ((packed));

extern struct gdt_desc64 gdt_desc;
extern uint64_t gdt[];

#endif
