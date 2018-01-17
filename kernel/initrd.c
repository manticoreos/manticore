#include <kernel/initrd.h>

#include <kernel/elf.h>
#include <kernel/page-alloc.h>
#include <kernel/printf.h>

void *initrd_start;
void *initrd_end;

void initrd_load(void)
{
	if (!initrd_start) {
		printf("No initrd found.\n");
		return;
	}
	printf("Found initrd at %x (%d bytes)\n", initrd_start, initrd_end - initrd_start);
}
