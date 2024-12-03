#include <e1000.h>
#include <printk.h>
#include <type.h>
#include <mode.h>
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

	while(e1000_transmit(txpacket, length) == 0)
	{
		printl("send block\n");
		#ifdef TXQE_TEST
		uint32_t mask = 0x0004010a;
		e1000_write_reg(e1000, E1000_TCTL, mask);
		#endif
		do_block(&current_running->list, &send_block_queue);
	}

	return length;  // Bytes it has transmitted
}

int do_net_recv(void *rxbuffer, int pkt_num, int *pkt_lens)
{
	// TODO: [p5-task2] Receive one network packet via e1000 device
	// TODO: [p5-task3] Call do_block when there is no packet on the way
	int i;
	uint64_t ret_length = 0;
	uint8_t * buffer = (uint8_t *)rxbuffer;
	for(i = 0; i < pkt_num; i++)
	{
		while((pkt_lens[i] = e1000_poll(buffer + ret_length)) == 0)
			do_block(&current_running->list, &recv_block_queue);
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

void check_send_recv()
{
	list_node_t * p = send_block_queue.next;
	while(p != &send_block_queue)
	{
		do_unblock(p);
	}
	p = recv_block_queue.next;
	while(p != &recv_block_queue)
	{
		do_unblock(p);
	}
}