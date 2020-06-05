#ifndef __NETINET_IN_H
#define __NETINET_IN_H

#include <sys/socket.h>

#include <inttypes.h>

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

#define AF_INET		2

#define INADDR_ANY	((in_addr_t) 0)
#define INADDR_NONE	((in_addr_t) -1)

struct in_addr {
	in_addr_t	s_addr;
};

struct sockaddr_in {
	sa_family_t	sin_family;
	in_port_t	sin_port;
	struct in_addr	sin_addr;
};

enum {
	IPPROTO_UDP	= 17,
};

#endif
