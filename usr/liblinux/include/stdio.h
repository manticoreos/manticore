#ifndef __STDIO_H
#define __STDIO_H

struct __file {
};

typedef struct __file *FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fprintf(FILE *stream, const char *format, ...);

#endif
