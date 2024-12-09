#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ETH_ALEN 6u
// Length of MAC address
#define ETH_P_IP 0x0800u
#define ETH_P_MAGIC 0x88b5u
static const uint8_t enetaddr[6] = {0x00, 0x0a, 0x35, 0x00, 0x1e, 0x53};
struct ethhdr
{
    uint8_t ether_dmac[ETH_ALEN];
    uint8_t ether_smac[ETH_ALEN];
    uint16_t ether_type;
};
struct package
{
    struct ethhdr eth;
    uint16_t magic;
    uint32_t data[(1280 - sizeof(struct ethhdr) - 2) / 4];
};
struct package pkg[3];

const int PKG_NUM = (1024 * 1024) / sizeof(pkg[0].data) + 1;

int main(void)
{
    int print_location = 1;
    uint64_t checksum = 0;

    sys_move_cursor(0, print_location);
    printf("> [RECV] start recv package.               \n");
    int start_tick = 0;
    int pkg_num = 0;
    int pkt_lens;
    while (pkg_num < PKG_NUM)
    {
        sys_net_recv(&pkg, 1, &pkt_lens);
        if (pkg[0].magic != ETH_P_MAGIC)
            continue;
        if (start_tick == 0)
            start_tick = sys_get_tick();
        for (int i = 0; i < sizeof(pkg[0].data) / 4; i++)
        {
            checksum += pkg[0].data[i];
        }
        /* printf("Recv %d package.\n", pkg_num);
        sys_move_cursor(0, print_location); */
        pkg_num++;
    }
    int end_tick = sys_get_tick();
    printf("Recv 1M package finished.\n");
    printf("Total checksum: %ld\n", checksum);
    printf("Total ticks: %d\n", end_tick - start_tick);
    printf("Total time: %d s\n", (end_tick - start_tick) / sys_get_timebase());

    return 0;
}