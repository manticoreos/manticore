#include <string.h>

size_t strnlen(const char *s, size_t maxlen)
{
	const char *p = memchr(s, 0, maxlen);
	if (!p) {
		return maxlen;
	}
	return p - s;
}
