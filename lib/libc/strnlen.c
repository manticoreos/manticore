#include <string.h>

/// strnlen method implements the length of string
/// \param s      s holds source string address
/// \param maxlen holds maximum length of string
/// \return       the string length of s
size_t strnlen(const char *s, size_t maxlen)
{
	const char *p = memchr(s, 0, maxlen);
	if (!p) {
		return maxlen;
	}
	return p - s;
}
