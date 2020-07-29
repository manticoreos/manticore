#ifndef __LIBLINUX_NETINET_IP_H
#define __LIBLINUX_NETINET_IP_H

#include <stdint.h>

#define IP_RF 0x8000U	   /* Reserved Flag (RF) */
#define IP_DF 0x4000U	   /* Don't Fragment (DF) */
#define IP_MF 0x2000U	   /* More Fragments (MF) */
#define IP_OFFMASK 0x1fffU /* Fragment offset mask */

struct iphdr {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	unsigned ihl : 4;
	unsigned version : 4;
#else
	unsigned version : 4;
	unsigned ihl : 4;
#endif
	uint8_t tos;
	uint16_t tot_len;
	uint16_t id;
	uint16_t frag_off;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t check;
	uint32_t saddr;
	uint32_t daddr;
};

#endif
