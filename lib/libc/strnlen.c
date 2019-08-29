#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// \breif Implementation of string length
//
// \params string pointer and maximum length
// \returns the string length of variable string
///////////////////////////////////////////////////////////////////////////////

size_t strnlen(const char *s, size_t maxlen)
{
	const char *p = memchr(s, 0, maxlen);
	if (!p) {
		return maxlen;
	}
	return p - s;
}
