#include <arpa/inet.h>

uint32_t htonl(uint32_t hostlong)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return hostlong;
#else
	return __builtin_bswap32(hostlong);
#endif
}

uint16_t htons(uint16_t hostshort)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return hostshort;
#else
	return __builtin_bswap16(hostshort);
#endif
}

uint32_t ntohl(uint32_t netlong)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return netlong;
#else
	return __builtin_bswap16(netlong);
#endif
}

uint16_t ntohs(uint16_t netshort)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return netshort;
#else
	return __builtin_bswap16(netshort);
#endif
}
