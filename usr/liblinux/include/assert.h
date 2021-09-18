#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdlib.h>
#include <stdio.h>

void __assert_fail(const char *expr, const char *file, unsigned int line, const char *function);

#define assert(expr)                                                        \
	do {                                                                \
		if (!(expr)) {                                              \
			__assert_fail(#expr, __FILE__, __LINE__, __func__); \
		}                                                           \
	} while (0)

#endif
