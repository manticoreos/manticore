#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdlib.h>
#include <stdio.h>

#define assert(expr)                                                                                                   \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			fprintf(stderr, "%s:%d: %s: Assertion `" #expr "' failed.\n", __FILE__, __LINE__, __func__);   \
			exit(1);                                                                                       \
		}                                                                                                      \
	} while (0)

#endif
