#ifndef __STDIO_H
#define __STDIO_H

#include <stddef.h>

struct __file {
};

typedef struct __file *FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fprintf(FILE *stream, const char *format, ...);

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif
