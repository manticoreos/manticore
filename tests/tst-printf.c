#include <kernel/printf.h>

#include <limits.h>

#define PRINTF(X...)                                                                                                   \
	do {                                                                                                           \
		printf("printf(%s) -> \"", #X);                                                                        \
		printf(X);                                                                                             \
		printf("\"\n");                                                                                        \
	} while (0)

void test_printf(void)
{
	PRINTF("%p", test_printf);
	PRINTF("%d", (int)INT_MAX);
	PRINTF("%d", (int)INT_MIN);
	PRINTF("%u", (unsigned int)UINT_MAX);
	PRINTF("%8d", (int)1);
	PRINTF("%8.8d", (int)1);
	PRINTF("%-8d", (int)1);
	PRINTF("%08d", (int)1);
	PRINTF("%x", (int)INT_MAX);
	PRINTF("%x", (int)INT_MIN);
	PRINTF("%x", (unsigned int)UINT_MAX);
	PRINTF("%X", (int)INT_MAX);
	PRINTF("%X", (int)INT_MIN);
	PRINTF("%X", (unsigned int)UINT_MAX);
	PRINTF("%ld", (long)LONG_MAX);
	PRINTF("%ld", (long)LONG_MIN);
	PRINTF("%lu", (unsigned long)ULONG_MAX);
	PRINTF("%lx", (long)LONG_MAX);
	PRINTF("%lx", (long)LONG_MIN);
	PRINTF("%lx", (unsigned long)ULONG_MAX);
	PRINTF("%lX", (long)LONG_MAX);
	PRINTF("%lX", (long)LONG_MIN);
	PRINTF("%lX", (unsigned long)ULONG_MAX);
	PRINTF("%lld", (long long)LLONG_MAX);
	PRINTF("%lld", (long long)LLONG_MIN);
	PRINTF("%llu", (unsigned long long)ULLONG_MAX);
	PRINTF("%llx", (long long)LLONG_MAX);
	PRINTF("%llx", (long long)LLONG_MIN);
	PRINTF("%llx", (unsigned long long)ULLONG_MAX);
	PRINTF("%llX", (long long)LLONG_MAX);
	PRINTF("%llX", (long long)LLONG_MIN);
	PRINTF("%llX", (unsigned long long)ULLONG_MAX);

	char str[1024];
	sprintf(str, "%s, %s", "hello", "world");
	puts(str);
}
