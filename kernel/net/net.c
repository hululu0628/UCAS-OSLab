#include <e1000.h>
#include <printk.h>
#include <type.h>
#include <os/proc.h>
#include <os/string.h>
#include <os/list.h>
#include <os/smp.h>

static LIST_HEAD(send_block_queue);
static LIST_HEAD(recv_block_queue);

int do_net_send(void *txpacket, int length)
{
	// TODO: [p5-task1] Transmit one network packet via e1000 device

	// TODO: [p5-task3] Call do_block when e1000 transmit queue is full
	// TODO: [p5-task4] Enable TXQE interrupt if transmit queue is full

	uint8_t * txp = (uint8_t *)txpacket;
	for(int i = 0; i < length; i += TX_PKT_SIZE)
	{
		while(e1000_transmit(txp + i, (length - i <= TX_PKT_SIZE)?(length - i):TX_PKT_SIZE,
				 (length - i <= TX_PKT_SIZE)?0:1) == 0)
		{
			do_block(&current_running->list, &send_block_queue);
		}
	}

	return length;  // Bytes it has transmitted
}

int do_net_recv(void *rxbuffer, int pkt_num, int *pkt_lens)
{
	// TODO: [p5-task2] Receive one network packet via e1000 device
	// TODO: [p5-task3] Call do_block when there is no packet on the way
	int i;
	uint64_t ret_length = 0;
	for(i = 0; i < pkt_num; i++)
	{
		pkt_lens[i] = e1000_poll(&rxbuffer[i]);
		ret_length += pkt_lens[i];
	}
	return ret_length;  // Bytes it has received
}

void handle_e1000_txqe()
{
	freeQueueToReady(&send_block_queue);
}

void handle_e1000_rxdmt0()
{
	freeQueueToReady(&recv_block_queue);
}

void net_handle_irq(void)
{
	// TODO: [p5-task4] Handle interrupts from network device
	uint32_t icr = e1000_read_reg(e1000, E1000_ICR);
	if(icr & E1000_ICR_TXQE)
		handle_e1000_txqe();
	else if(icr & E1000_ICR_RXDMT0)
		handle_e1000_rxdmt0();
	else
		printl("WARNING: Unknown E1000 interrupt\n");
}

void check_send()
{
	list_node_t * p = send_block_queue.next;
	while(p != &send_block_queue)
	{
		do_unblock(p);
	}
}