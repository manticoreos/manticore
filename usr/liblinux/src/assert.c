#include <assert.h>

void __assert_fail(const char *expr, const char *file, unsigned int line, const char *function)
{
	fprintf(stderr, "%s:%d: %s: Assertion `%s' failed.\n", file, line, expr, function);
	exit(1);
}
