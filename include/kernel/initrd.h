#ifndef KERNEL_INITRD_H
#define KERNEL_INITRD_H

extern void *initrd_start;
extern void *initrd_end;

void initrd_load(void);

#endif
