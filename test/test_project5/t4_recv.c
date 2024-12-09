#include <stdio.h>
#include <stdint.h>
#include <net.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syscall.h>

uint8_t recv_buffer[2048];
int length;

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

int main()
{
	uint64_t ret_len = 0;
	uint64_t temp;
	int i = 0;
	uint64_t begin,end,interval;
	uint16_t cs = 0;

	begin = clock();
	while(1)
	{
		temp = sys_net_recv(&recv_buffer, 1, &length);
		if(((pkt *)&recv_buffer)->data[0] != 0x42)
			continue;
		ret_len += temp;
		cs = checksum((uint16_t *)&recv_buffer, sizeof(eth_t) + MAX_PL_LEN, 0);
		/*if(cs != ((pkt *)&recv_buffer)->checksum)
		{
			sys_move_cursor(0, 2);
			printf("pkt: %d, checksum: 0x%x\n", i, cs);
		}*/
		i++;
		if((i % 50 == 0) && i != 0)
		{
			end = clock();
			interval = end - begin;
			sys_move_cursor(0, 3);
			printf("pkt number: %d, %d B/s", i, (ret_len * CLOCKS_PER_SEC) / interval);
			ret_len = 0;
			begin = clock();
		}
	}
	return 0;
}