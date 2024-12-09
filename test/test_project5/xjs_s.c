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
struct package pkg;
const int PGK_NUM = (1024 * 1024) / sizeof(pkg.data) + 1;

int main(void)
{
    int print_location = 0;

    sys_move_cursor(0, print_location);
    printf("> [SPEED SEND] start send 1M package.               \n");

    uint32_t index = 0;
    uint64_t checksum = 0;
    int start_tick = sys_get_tick();
    for (int pkg_num = 0; pkg_num < PGK_NUM; pkg_num++)
    {
        for (int i = 0; i < ETH_ALEN; i++)
        {
            pkg.eth.ether_dmac[i] = 0xff;
            pkg.eth.ether_smac[i] = enetaddr[i];
        }
        pkg.eth.ether_type = ETH_P_IP;
        pkg.magic = ETH_P_MAGIC;

        for (int i = 0; i < sizeof(pkg.data) / 4; i++)
        {
            pkg.data[i] = index * index;
            checksum += pkg.data[i];
            index++;
            if (index > 10000)
                index = 0;
        }
        sys_net_send(&pkg, 1280);
        /* printf("Send %d package.\n", index / sizeof(pkg.data));
        sys_move_cursor(0, print_location); */
    }
    int end_tick = sys_get_tick();
    printf("Send 1M package finished.\n");
    printf("Total checksum: %ld\n", checksum);
    printf("Total ticks: %d\n", end_tick - start_tick);
    printf("Total time: %d s\n", (end_tick - start_tick) / sys_get_timebase());
    return 0;
}