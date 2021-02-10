#ifndef __STDLIB_H
#define __STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);

void abort(void);

void exit(int status);

#endif
