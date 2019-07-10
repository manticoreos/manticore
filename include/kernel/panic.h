#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

void panic(const char *fmt, ...) __attribute__((__format__(__printf__, 1, 2)));

#endif
