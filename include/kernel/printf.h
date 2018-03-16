#ifndef KERNEL_PRINTF_H
#define KERNEL_PRINTF_H

#include <stddef.h>
#include <stdarg.h>

int vprintf(const char *fmt, va_list ap);
int printf(const char *fmt, ...) __attribute__((__format__(__printf__, 1, 2)));
int putchar(int c);
int puts(const char *s);
int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
int vsprintf(char *str, const char *fmt, va_list ap);
int sprintf(char *str, const char *fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
int snprintf(char *str, size_t size, const char *fmt, ...) __attribute__((__format__(__printf__, 3, 4)));

#endif
