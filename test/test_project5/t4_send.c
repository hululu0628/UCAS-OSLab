#include <stdio.h>
#include <stdint.h>
#include <net.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syscall.h>

#define BUFFER_SIZE 4

pkt send_buffer[BUFFER_SIZE];

static inline uint16_t checksum(uint16_t *ptr, int nbytes, uint32_t sum)
{
	if (nbytes % 2) {
		sum += ((uint8_t *)ptr)[--nbytes];
	}

	while (nbytes > 0) {
		sum += *ptr++;
		nbytes -= 2;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);

	return (uint16_t)~sum;
}

int main(void)
{
	int i, j, k;
	srand(clock());

	for(i = 0; i < BUFFER_SIZE; i++)
	{
		for(j = 0; j < ETH_ALEN; j++)
		{
			send_buffer[i].eth_hd.dst[j] = 0xff;
			send_buffer[i].eth_hd.src[j] = enetaddr[j];
		}
		send_buffer[i].eth_hd.ether_type = ETH_P_IP;
		for(j = 0; j < BUFFER_SIZE; j++)
		{
			for(k = 0; k < MAX_PL_LEN; k++)
			{
				srand(clock());
				send_buffer[j].data[k] = 0xff & (uint8_t)rand();
			}
			send_buffer[j].data[0] = 0x42;		// Magic Number
		}
		send_buffer[i].checksum = checksum((uint16_t *)&send_buffer[i], sizeof(eth_t) + MAX_PL_LEN, 0);
		printf("Buffer %d checksum: 0x%x\n", i, send_buffer[i].checksum);
	}
	
	for(i = 0; i < 300; i++)
	{
		for(j = 0; j < BUFFER_SIZE; j++)
		{
			sys_net_send(&send_buffer[j], sizeof(pkt));
		}
	}
	printf("Finish\n");
	while(1);
	return 0;
}