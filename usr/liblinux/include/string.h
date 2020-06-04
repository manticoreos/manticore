#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

void *memset(void *s, int c, size_t len);
void *memcpy(void *dst, const void *src, size_t n);
char *strerror(int errnum);
size_t strlen(const char *s);

#endif
