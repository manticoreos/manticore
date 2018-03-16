#include <kernel/initrd.h>

#include <kernel/elf.h>
#include <kernel/page-alloc.h>
#include <kernel/printf.h>

void *initrd_start;
void *initrd_end;

extern void process_run(const char *image_start, unsigned long image_size);

void initrd_load(void)
{
	if (!initrd_start) {
		printf("No initrd found.\n");
		return;
	}
	unsigned long initrd_size = initrd_end - initrd_start;

	printf("Found initrd at %p (%lu bytes)\n", initrd_start, initrd_size);

	process_run(initrd_start, initrd_size);
}
