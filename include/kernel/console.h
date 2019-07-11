#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include <stddef.h>

void console_init(void);
void console_write_char(char ch);
void console_write_str(const char *s);
void console_write(const char *s, size_t count);

#endif
