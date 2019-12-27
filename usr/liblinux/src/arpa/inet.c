#include <arpa/inet.h>

#include <limits.h>

in_addr_t inet_addr(const char *cp)
{
	uint32_t ret = 0, acc = 0;

	/* FIXME: Fail if "cp" is not a valid IPv4 address. */

	for (;;) {
		int c = *cp++;

		switch (c) {
		case '0' ... '9': {
			int digit = c - '0';

			acc = acc * 10 + digit;

			if (acc > UCHAR_MAX) {
				return INADDR_NONE;
			}
			break;
		}
		case '.': {
			ret = (ret << 8) | acc;
			acc = 0;
			break;
		}
		case '\0': {
			ret = (ret << 8) | acc;
			return htonl(ret);
		}
		default:
			return INADDR_NONE;
		}
	}
}
