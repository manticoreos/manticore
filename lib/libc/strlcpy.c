#include <string.h>

size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);
	if (size > 0) {
		size_t len = ret;
		if (len >= size) {
			len = size - 1;
		}
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}
