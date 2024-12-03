#include <os/list.h>
#include <os/proc.h>
#include <io.h>
#include <os/mm.h>
#include <e1000.h>
#include <type.h>
#include <os/string.h>
#include <os/time.h>
#include <assert.h>
#include <pgtable.h>
#include <mode.h>

// E1000 Registers Base Pointer
// what you get in main.c is physical address, but need virtual address
volatile uint8_t *e1000;  

// E1000 Tx & Rx Descriptors
static struct e1000_tx_desc tx_desc_array[TXDESCS] __attribute__((aligned(16)));
static struct e1000_rx_desc rx_desc_array[RXDESCS] __attribute__((aligned(16)));

// E1000 Tx & Rx packet buffer
static char tx_pkt_buffer[TXDESCS][TX_PKT_SIZE];
static char rx_pkt_buffer[RXDESCS][RX_PKT_SIZE];

// Fixed Ethernet MAC Address of E1000
static const uint8_t enetaddr[6] = {0x00, 0x0a, 0x35, 0x00, 0x1e, 0x53};


static void init_desc_array(void)
{
	int i;
	for(i = 0; i < TXDESCS; i++)
	{
		tx_desc_array[i].status = E1000_TXD_STAT_DD;
	}
	for(i = 0; i < RXDESCS; i++)
	{
		rx_desc_array[i].addr = kva2pa((uint64_t)&rx_pkt_buffer[i]);
		rx_desc_array[i].status = 0;
	}
}

static void e1000_irq_init(void)
{
	e1000_write_reg(e1000, E1000_IMS, E1000_IMS_RXDMT0 | E1000_IMS_TXQE);
}


/**
 * e1000_reset - Reset Tx and Rx Units; mask and clear all interrupts.
 **/
static void e1000_reset(void)
{
		/* Turn off the ethernet interface */
	e1000_write_reg(e1000, E1000_RCTL, 0);
	e1000_write_reg(e1000, E1000_TCTL, 0);

		/* Clear the transmit ring */
	e1000_write_reg(e1000, E1000_TDH, 0);
	e1000_write_reg(e1000, E1000_TDT, 0);

		/* Clear the receive ring */
	e1000_write_reg(e1000, E1000_RDH, 0);
	e1000_write_reg(e1000, E1000_RDT, 0);

		/**
	* Delay to allow any outstanding PCI transactions to complete before
		* resetting the device
		*/
	latency(1);

		/* Clear interrupt mask to stop board from generating interrupts */
	e1000_write_reg(e1000, E1000_IMC, 0xffffffff);

	/* Clear any pending interrupt events. */
	while (0 != e1000_read_reg(e1000, E1000_ICR)) ;
}

/**
 * e1000_configure_tx - Configure 8254x Transmit Unit after Reset
 **/
static void e1000_configure_tx(void)
{
	/* TODO: [p5-task1] Initialize tx descriptors */
	/* TODO: [p5-task1] Set up the Tx descriptor base address and length */
	/* TODO: [p5-task1] Set up the HW Tx Head and Tail descriptor pointers */
	/* TODO: [p5-task1] Program the Transmit Control Register */

	// set up TDBAL & TDBAH, point to base addr of descriptor array 
	e1000_write_reg(e1000, E1000_TDBAL, 
		(uint32_t)(kva2pa((uint64_t)tx_desc_array) & 0xffffffff));
	e1000_write_reg(e1000, E1000_TDBAH, 
		(uint32_t)(kva2pa((uint64_t)tx_desc_array) >> 32lu));
	// set up TDLEN, array length
	e1000_write_reg(e1000, E1000_TDLEN, TXDESCS * sizeof(struct e1000_tx_desc));

	// set up TDH & TDT, head & tail to the array
	e1000_write_reg(e1000, E1000_TDH, 0);
	e1000_write_reg(e1000, E1000_TDT, 0);

	// set up TCTL. for details, see guidebook
	// EN = 1; PSP = 1; CT = 10H, COLD = 40H (0b100_0000_0001_0000_1010)
	uint32_t mask = 0x0004010a;
	#ifndef TXQE_TEST
	e1000_write_reg(e1000, E1000_TCTL, mask);
	#endif

	printl("TX reg:\n"
		"TDBAL: %lx, TDBAH: %lx, TDLEN: %lx\n"
		"TDH: %lx, TDT: %lx\n",
		e1000_read_reg(e1000, E1000_TDBAL),
		e1000_read_reg(e1000, E1000_TDBAH),
		e1000_read_reg(e1000, E1000_TDLEN),
		e1000_read_reg(e1000, E1000_TDH),
		e1000_read_reg(e1000, E1000_TDT));
}

/**
 * e1000_configure_rx - Configure 8254x Receive Unit after Reset
 **/
static void e1000_configure_rx(void)
{
	/* TODO: [p5-task2] Set e1000 MAC Address to RAR[0] */
	// Recieve Address
	// recieve boardcast data
	e1000_write_reg_array(e1000, E1000_RA, 1, 
		E1000_RAH_AV | enetaddr[5] << 8 | enetaddr[4]);
	// fill MAC
	e1000_write_reg_array(e1000, E1000_RA, 0, 
		enetaddr[3] << 24 | enetaddr[2] << 16 | enetaddr[1] << 8 | enetaddr[0]);
	/* TODO: [p5-task2] Initialize rx descriptors */
	/* TODO: [p5-task2] Set up the Rx descriptor base address and length */
	e1000_write_reg(e1000, E1000_RDBAL, 
		(uint32_t)(kva2pa((uint64_t)rx_desc_array) & 0xffffffff));
	e1000_write_reg(e1000, E1000_RDBAH, 
		(uint32_t)(kva2pa((uint64_t)rx_desc_array) >> 32lu));
	e1000_write_reg(e1000, E1000_RDLEN, RXDESCS * sizeof(struct e1000_rx_desc));
	/* TODO: [p5-task2] Set up the HW Rx Head and Tail descriptor pointers */
	e1000_write_reg(e1000, E1000_RDH, 0);
	e1000_write_reg(e1000, E1000_RDT, RXDESCS - 1);
	/* TODO: [p5-task2] Program the Receive Control Register */
	uint32_t mask = 0x00008002;
	e1000_write_reg(e1000, E1000_RCTL, mask);
	/* TODO: [p5-task4] Enable RXDMT0 Interrupt */
}

/**
 * e1000_init - Initialize e1000 device and descriptors
 **/
void e1000_init(void)
{
	/* Reset E1000 Tx & Rx Units; mask & clear all interrupts */
	e1000_reset();

	/* Configure E1000 Tx Unit */
	e1000_configure_tx();

	/* Configure E1000 Rx Unit */
	e1000_configure_rx();

	init_desc_array();

	e1000_irq_init();
}

/**
 * e1000_transmit - Transmit packet through e1000 net device
 * @param txpacket - The buffer address of packet to be transmitted
 * @param length - Length of this packet
 * @return - Number of bytes that are transmitted successfully
 **/
int e1000_transmit(void *txpacket, int length)
{
	/* TODO: [p5-task1] Transmit one packet from txpacket */
	static int len = 0;
	uint32_t tail, tail_next;
	struct e1000_tx_desc t;

	int eop_flag = 0;

	len = (len == 0) ? length : len;

	while(1)
	{
		tail = e1000_read_reg(e1000, E1000_TDT);
		tail_next = (tail + 1) % TXDESCS;
		eop_flag = (len <= TX_PKT_SIZE);

		if(!(tx_desc_array[tail_next].status & E1000_TXD_STAT_DD))
			return 0;

		t.addr = kva2pa((uint64_t)&tx_pkt_buffer[tail]);
		t.length = length;
		t.cmd = E1000_TXD_CMD_RS;
		t.cso = 0; t.css = 0; t.special = 0; t.status = 0;
		if(eop_flag)
			t.cmd |= E1000_TXD_CMD_EOP;

		tx_desc_array[tail] = t;

		memcpy((uint8_t *)&tx_pkt_buffer[tail], txpacket + length - len, 
							eop_flag ? len : TX_PKT_SIZE);

		e1000_write_reg(e1000, E1000_TDT, (tail+1) % TXDESCS);

		local_flush_dcache();

		if(eop_flag)
		{
			len = 0;
			break;
		}

		len -= TX_PKT_SIZE;
	}
	
	return length;
}

/**
 * e1000_poll - Receive packet through e1000 net device
 * @param rxbuffer - The address of buffer to store received packet
 * @return - Length of received packet
 **/
int e1000_poll(void *rxbuffer)
{
	/* TODO: [p5-task2] Receive one packet and put it into rxbuffer */
	static int len = 0;
	int ret_len = 0;
	uint8_t * buffer = (uint8_t *)rxbuffer;
	uint32_t tail,tail_next;
	while(1)
	{
		tail = e1000_read_reg(e1000, E1000_RDT);
		tail_next = (tail + 1) % RXDESCS;

		if(!(rx_desc_array[tail_next].status & E1000_RXD_STAT_DD))
			return 0;

		memcpy(buffer + len, (uint8_t *)&rx_pkt_buffer[tail_next], rx_desc_array[tail_next].length);
		len += rx_desc_array[tail_next].length;

		rx_desc_array[tail_next].status ^= E1000_RXD_STAT_DD;

		e1000_write_reg(e1000, E1000_RDT, (tail+1) % RXDESCS);
		
		local_flush_dcache();

		if(rx_desc_array[tail_next].status & E1000_RXD_STAT_EOP)
		{
			ret_len = len;
			len = 0;
			return ret_len;
		}
	}
	return 0;
}
