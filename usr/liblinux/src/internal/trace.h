#ifndef __LIBLINUX_INTERNAL_TRACE_H
#define __LBILINUX_INTERNAL_TRACE_H

#include <stdio.h>

#ifdef __LIBLINUX_TRACING
#  define LIBLINUX_TRACE(func) do {				\
	fprintf(stderr, "liblinux -> %s\n", #func);		\
} while (0)
#else
#  define LIBLINUX_TRACE(func)
#endif

#endif
