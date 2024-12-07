#ifndef INCLUDE_NET_H
#define INCLUDE_NET_H

#include <stdint.h>

#define ETH_ALEN 6
#define ETH_P_IP 0x0800u

#define MAX_PL_LEN 1024

const uint8_t enetaddr[6] = {0x00, 0x0a, 0x35, 0x00, 0x1e, 0x53};

typedef struct eth_t
{
	uint8_t dst[ETH_ALEN];
	uint8_t src[ETH_ALEN];
	uint16_t ether_type;
}eth_t;

typedef struct pkt{
	eth_t eth_hd;
	uint8_t data[MAX_PL_LEN];
	uint16_t checksum;
}pkt;

#endif