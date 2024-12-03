#ifndef __INCLUDE_NET_H__
#define __INCLUDE_NET_H__

#include <os/list.h>
#include <type.h>

#define PKT_NUM 32

#define ETH_ALEN 6u                 // Length of MAC address
#define ETH_P_IP 0x0800u            // IP protocol
// Ethernet header
struct ethhdr {
    uint8_t ether_dmac[ETH_ALEN];   // destination mac address
    uint8_t ether_smac[ETH_ALEN];   // source mac address
    uint16_t ether_type;            // protocol format
};

extern void net_handle_irq(void);
extern int do_net_recv(void *rxbuffer, int pkt_num, int *pkt_lens);
extern int do_net_send(void *txpacket, int length);

extern void handle_e1000_txqe(void);
extern void handle_e1000_rxdmt0(void);
extern void check_send(void);
#endif  // !__INCLUDE_NET_H__
