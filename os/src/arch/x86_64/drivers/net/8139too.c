#include <stdint-gcc.h>
#include "drivers/net/8139too.h"
#include "drivers/net/arp/arp.h"
#include "drivers/pci/pci.h"
#include "drivers/memory/memoryManager.h"
#include "drivers/interrupts/idt.h"
#include "utils/printk.h"
#include "utils/utils.h"
#include "utils/byte_order.h"
#include "ethernet/ethernet.h"

/* RealTek RTL-8139 Fast Ethernet driver */
static const unsigned int rtl8139_tx_config =
	TxIFG96 | (TX_DMA_BURST << TxDMAShift) | (TX_RETRY << TxRetryShift);

//todo clean this up, just using to test functionality inside of isr
uint32_t global_rt_ioaddr = 0;
uint64_t global_rx_buffer_virt = 0;
uint64_t global_rx_buffer_size = 0;
rt8139_private *global_rtl_priv = NULL;

void wipe_dma_buffer(uint64_t virt_addr, uint64_t size){
	uint64_t i = 0;
	uint8_t *buff = (uint8_t*)virt_addr;

	while(i < size){
		buff[i] = 0;
		i += 1;
	}
}

void print_ethertype(uint32_t ethertype){
	switch(ethertype){
	case ETHRTYPE_IPV4:
		printk("IPV4");
		return;
	case ETHRTYPE_ARP:
		printk("ARP");
		return;
	case ETHRTYPE_IPV6:
		printk("IPV6");
		return;
	default:
		printk("Unknown");
	}
}

void parse_eth_frame(uint8_t *eth_frame, unsigned int frame_size){
	/* uint8_t *dest_mac = eth_frame; */
	/* uint8_t *source_mac = eth_frame + 6; */
	uint16_t ethertype_or_len = ntohs(*((uint16_t*)(eth_frame + 12)));

        /* hw_addr source_hw_addr = create_hw_addr(source_mac); */
        /* hw_addr dest_hw_addr = create_hw_addr(dest_mac); */

	/* char *source_addr_str = hw_addr_to_str(source_hw_addr); */
	/* char *dest_addr_str = hw_addr_to_str(dest_hw_addr); */
	
	/* printk("****** Received Ethernet Frame ******\n"); */
	/* printk("Total Length: %u\n", frame_size); */
    
	/* printk("Dest MAC: %s\n", dest_addr_str); */
	/* printk("Source MAC: %s\n", source_addr_str); */

	if(ethertype_or_len < 1501){
		/* printk("Data Length: %hu\n", ethertype_or_len); */
	}
	else {
		/* printk("Ethertype: "); */
		/* print_ethertype(ethertype_or_len); */
		/* printk(" (0x%04x)\n", ethertype_or_len); */
		if(ethertype_or_len == ETHRTYPE_ARP){
			handle_received_arp_packet(eth_frame, frame_size);
		}
	}
    
	/* printk("*************************************\n\n"); */

	/* kfree(source_addr_str); */
	/* kfree(dest_addr_str); */
}

/* Inform rtl8139 that packet was successfully received 
 * NOTE: acknowledgement procedure taken from Linux Source
 */
static void rtl8139_rx_isr_ack(uint32_t rt_ioaddr){
	static uint64_t rx_errors = 0, rx_fifo_errors = 0;
	uint16_t status;

	status = inw(rt_ioaddr + IntrStatus) & RxAckBits;

	/* clear out errors and receive interrupts 
	 * NOTE: logic taken from linux source (error checking seems to only be
	 *       used for statistical purposes, which I don't currently support) */
	if(status != 0){
		if(status & (RxFIFOOver | RxOverflow)) {
			printk_warn("Rx Error occurred (#%lx during driver operation)\n", ++rx_errors);
			if (status & RxFIFOOver){
				printk_warn("Rx Fifo Error occurred (#%lx during driver operation\n", ++rx_fifo_errors);
			}
		}

		// write RxAck to rtl8139's interrupt status register
		outw(global_rt_ioaddr + IntrStatus, RxAckBits);
	}
}

int rtl_rx_interrupt(){
	int received = 0;
	uint32_t rt_ioaddr = global_rt_ioaddr;
	rt8139_private *priv = global_rtl_priv;
	uint8_t *rx_buf = (uint8_t*)priv->rx_buf_virt;
	uint64_t cur_rx = priv->cur_rx;
	uint64_t rx_buf_size = priv->rx_buf_size;

	unsigned int rx_size = 0;
	uint32_t ring_offset;
	uint32_t rx_status;
	uint8_t *eth_frame_beginning;
	
	if(global_rtl_priv == NULL){
		printk_err("rtl not initialized, can't receive packet\n");
		asm("hlt");
	}
    
	//loop until rx buffer is empty
	while((inb(rt_ioaddr + ChipCmd) & RxBufEmpty) == 0){
		ring_offset = cur_rx % rx_buf_size;
		//unsigned int pkt_size;
	
		rx_status = le32_to_cpu(*(uint32_t*) (rx_buf + ring_offset));
		rx_size = rx_status >> 16;

		//pkt_size = rx_size;

		// packet copy from FIFO still in progress. Theoretically should never happen since EarlyRx is disabled.
		if(rx_size == 0xfff0){
			printk_info("fifo copy in progress, aborting\n");
			break;
		}
	
		/* if rx err or invalid rx_size/rx_status received 
		 * (which happens if we get lost in the ring), 
		 * Rx process gets reset, so we abort any further Rx processing */
		if(rx_size > (MAX_ETH_FRAME_SIZE + 4) ||
		   (rx_size < 8) ||
		   (!(rx_status & RxStatusOK))){
			//handle bad status (if size is ok)
			if((rx_size <= (MAX_ETH_FRAME_SIZE + 4)) &&
			   (rx_size >= 8) &&
			   (!(rx_status & RxStatusOK))) {
				if(rx_status & RxCRCErr) {
					printk_warn("Rx Checksum err\n");
					goto keep_pkt;
				}
				if(rx_status & RxRunt){
					printk_warn("Rx length err\n");
					goto keep_pkt;
				}
				printk_err("Rx unrecognized Status Error\n");
			}
			else {
				printk_err("bad packet size: %u\n", rx_size);
			}

			printk_err("unrecoverable Rx error, halting (should be resetting nic)\n");
			printk_err("debug_info:\n\trx_ring_beginning=0x%lx\n\trx_ring_end=0x%lx\n\tcur_rx=0x%lx\n", rx_buf, rx_buf + rx_buf_size, rx_buf + cur_rx);
			asm("hlt");

			received = -1;
			goto out;
		}

	keep_pkt:
		received++;

		//move to next packet (logic taken from linux source, assuming that this properly aligns cur_rx to the next packet)
		cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
		outw(rt_ioaddr + RxBufPtr, (uint16_t) (cur_rx - 16));

		//handle received ethernet frame
		//todo: currently only printing information about packet, implement logic to store received transmission in the kernel
		eth_frame_beginning = (uint8_t*)(rx_buf + ring_offset + 4);
		parse_eth_frame(eth_frame_beginning, rx_size);
	
		//acknowledge packet
		rtl8139_rx_isr_ack(rt_ioaddr);
	}

	//update device's rx ring buffer index
	priv->cur_rx = cur_rx;
	
out:
	return received;
}

static void rtl8139_tx_interrupt(){
	uint32_t rt_ioaddr = global_rt_ioaddr;
	rt8139_private *priv = global_rtl_priv;

	//todo change this (will only clear the most recent transmission)
	unsigned long tx_left = 1;

	while(tx_left > 0){
		int entry = (priv->cur_tx - 1) % NUM_TX_DESC;
		int txstatus;

		txstatus = inl(rt_ioaddr + (TxStatus0 + (entry * sizeof(uint32_t))));

		if (!(txstatus & (TxStatOK | TxUnderrun | TxAborted))){
			break; /* It sill hasn't been txed */
		}

		if (txstatus & (TxOutOfWindow | TxAborted)) {
			/* There was a major error, log it. */
			printk_warn("major tx eroror\n");

			if(txstatus & TxAborted){
				printk_err("Tx aborted error\n");
				outl(rt_ioaddr + TxConfig, TxClearAbt);
				outw(rt_ioaddr + IntrStatus, TxErr);
			}
			asm("hlt");
		}
		else {
			if (txstatus & TxUnderrun){
				printk_err("TxUnderrun, don't know how to handle, halting\n");
				asm("hlt");
			}
			
		}
		
		tx_left -= 1;
	}
}

/* interrupt service routine for rx/tx interrupt 
 * NOTE: heavy lifting occurs in `rtl_rx_interrupt` function 
 * TODO: figure out how to make this work without the use of global variables */
void rtl_isr(int irq, int err){
	uint16_t status, ackstat;
	uint32_t rt_ioaddr = global_rt_ioaddr;
	//rt8139_private *priv = global_rtl_priv;
	
	//printk("received rtl interrupt\n", irq);
	
	status = inw(rt_ioaddr + IntrStatus);
	ackstat = status & ~(RxAckBits | TxErr);

	//todo not really sure what this does....
	if(ackstat){
		outw(rt_ioaddr + IntrStatus, ackstat);
	}

	if(status & RxAckBits){
		rtl_rx_interrupt();
		//printk_info("received %d packets\n", received);
	}
	
	if (status & (TxOK | TxErr)) {
		rtl8139_tx_interrupt();

		if (status & TxErr)
			outw(rt_ioaddr + IntrStatus, TxErr);
	}

}

void rtl8139_wake_up(uint32_t rt_ioaddr){
	uint8_t tmp8;
	uint8_t new_tmp8;


	new_tmp8 = tmp8 = inb(rt_ioaddr + Config1);
	new_tmp8 |= Cfg1_PM_Enable;

	if(/*HasLWake && */new_tmp8 & LWAKE){
		printk_info("setting lwake\n");
		new_tmp8 &= ~LWAKE;
	}
	else {
		printk_info("ignoring LWAKE\n");
	}
    
	if(new_tmp8 != tmp8){
		printk_info("setting new config1\n");
		outb(rt_ioaddr + Cfg9346, Cfg9346_Unlock);
		outb(rt_ioaddr + Config1, tmp8);
		outb(rt_ioaddr + Cfg9346, Cfg9346_Lock);    
	}
	else {
		printk_info("new_tmp8 same as old tmp8\n");
	}
	/*if(HasLWake && )*/
	tmp8 = inb(rt_ioaddr + Config4);
	if(tmp8 & LWPTN){
		printk_info("setting new config4\n");
		outb(rt_ioaddr + Cfg9346, Cfg9346_Unlock);
		outb(rt_ioaddr + Config1, tmp8 & ~LWPTN);
		outb(rt_ioaddr + Cfg9346, Cfg9346_Lock);
	}
}

void rtl8139_chip_reset(uint32_t rt_ioaddr){
	uint8_t cmd_reg;

	// send 0x10 to Command register
	printk_info("initializing software reset...");
	outb(rt_ioaddr + ChipCmd, CmdReset);

	// poll RST bit until not set to 1
	while(((cmd_reg = inb(rt_ioaddr + ChipCmd)) & CmdReset) != 0)
		printk("cmd_reg = 0x%x\n", cmd_reg);
    
	printk(" done!\n");
}

void print_rtl8139_info(rt8139_private *priv){
	uint8_t *mac_addr_bytes = (uint8_t*)&(priv->mac_addr);
	int i;
	
	printk_debug("+++++++++++++ RTL8139 INFORMATION +++++++++++++\n");
	
	printk_debug("* interrupt line: ");
	printk("0x%x\n", priv->dev->interrupt_line);
	
	printk_debug("* pci base ioaddr: ");
	printk("0x%x\n", priv->dev->io_addrs->base_addr);

	printk_debug("* MAC address: ");
	printk("%02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr_bytes[0],
	       mac_addr_bytes[1], mac_addr_bytes[2], mac_addr_bytes[3],
	       mac_addr_bytes[4], mac_addr_bytes[5]);

	printk_debug("* rx ring buffer:\n");
	printk("\tbegins at virtual address = 0x%lx (physical address = 0x%lx)\n", priv->rx_buf_virt, priv->rx_buf_phys);
	printk("\tsize with padding = 0x%lx\n", priv->rx_buf_total_size);
	printk("\tsize without padding = 0x%lx\n", priv->rx_buf_size);

	printk_debug("* tx buffers:\n");
	printk("\tnumber of tx buffers = %u (in contiguous physical memory starting at address = 0x%lx)\n", NUM_TX_DESC, priv->tx_buf_phys);
	for(i = 0; i < NUM_TX_DESC; i++){
		printk("\t %u) begins at virtual address = 0x%lx\n", i, priv->tx_bufs[i]);
	}
	printk("\tindividual tx buffer size = 0x%lx\n", MAX_ETH_FRAME_SIZE);
	printk("\ttotal size of all tx buffers = 0x%lx\n", TX_BUF_TOT_LEN);
	
	printk_debug("++++++++++++++++++++++++++++++++++++++++++++++\n");
}

int init_rt8139(PCIDevice *dev) {
	rt8139_private *priv;
	uint32_t command;
	uint32_t rt_ioaddr;
	uint64_t rx_buffer_virt;
	uint32_t rx_buffer_phys;
	uint8_t tmp;
	uint16_t intr_mask; 
	int i;

	//disable interrupts
	asm("CLI");

	printk_info("---- Performing initialization of the discovered rtl8139 ----\n");
	
	priv = (rt8139_private*)kmalloc(sizeof(rt8139_private));
	priv->dev = dev;

	IRQ_set_handler(dev->interrupt_line + 0x20, rtl_isr);
	IRQ_clear_mask(dev->interrupt_line);
    
	/* Enable PCI Bus Mastering */
	// read command register from the device's PCI Configuration space, set bit 2 (bus mastering bit), and write the modified command register
	printk_info("\tenabling bus mastering for rtl839\n");
	command = pci_config_read_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, 2);
	command |= 0b100; //enable bus mastering
	command |= 0b1; //enable I/O space
	command |= 0b10; //enable Memory space
	command |= 0b10000; //enable memory write
	//command &= 0b01111111111; //disable interrupt disable

	if(command & 0b10000000000){
		printk_info("\tinterrupts disabled\n");
	}
	else {
		printk_info("\tinterrupts enabled\n");
	}
    
	pci_config_write_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, command, 2);

	if(!(dev->io_addrs)){
		printk_err("Supplied rt8139 has no base i/o address, can't initialize\n");
		return 1;
	}
    
	rt_ioaddr = global_rt_ioaddr = dev->io_addrs->base_addr;

	/* turn on the device */
	//bring old chips out of low power mode
	outb(rt_ioaddr + HltClk, 'R');
    
	printk_info("\twaking up the chip\n");
	rtl8139_wake_up(rt_ioaddr);
    
	/* software reset */
	printk_info("\tperforming software reset\n");
	rtl8139_chip_reset(rt_ioaddr);

	uint64_t mac_addr_int = 0;
        mac_addr_int += le32_to_cpu(inl(rt_ioaddr + MAC0));
        mac_addr_int <<= 16;
        mac_addr_int += le32_to_cpu(inl(rt_ioaddr + MAC0 + 4));

	priv->mac_addr = create_hw_addr((uint8_t*)&mac_addr_int);

	//todo figure out how to actually set this up
        priv->ipv4_addr = 192;
	priv->ipv4_addr <<= 8;
        priv->ipv4_addr += 168;
        priv->ipv4_addr <<= 8;
        priv->ipv4_addr += 122;
        priv->ipv4_addr <<= 8;
        priv->ipv4_addr += 18;
	
	/* unlock Config[01234] and BMCR register writes */
	outb(rt_ioaddr + Cfg9346, Cfg9346_Unlock);
    
	/* initialize chip's receive buffer */
	// size of 8192 + 16 (8K + 16 bytes) is recommended
	global_rx_buffer_virt = rx_buffer_virt = (uint64_t)alloc_dma_coherent(RX_BUF_TOT_LEN, &rx_buffer_phys);
	wipe_dma_buffer(rx_buffer_virt, RX_BUF_TOT_LEN);
	
	printk_debug("\tallocated %lx bytes of memory (including padding) for rx buffer (physical start = %lx, virtual start = %lx)\n", RX_BUF_TOT_LEN, rx_buffer_phys, rx_buffer_virt);
	
	outl(rt_ioaddr + RxBuf, rx_buffer_phys);

	global_rx_buffer_size = priv->rx_buf_size = RX_BUF_LEN;
    
	priv->rx_buf_virt = rx_buffer_virt;
	priv->rx_buf_phys = rx_buffer_phys;
	priv->cur_rx = 0;
    
	/* enable receive and transmitter
	 * Start up the RX and TX functions.
	 * The RE (Receiver Enabled) and TE (Transmitter Enabled) bits are located in the command register (0x37). */
	outb(rt_ioaddr + ChipCmd, CmdRxEnb | CmdTxEnb);
    
	/* configure the receive buffer */
	outl(rt_ioaddr + RxConfig, 0xf | (1 << 7)); //todo this can be improved
	outl(rt_ioaddr + TxConfig, rtl8139_tx_config);

	//rtl_check_media(dev,1);
    
	/* lock Config[01234] and BMCR register writes */
	outb(rt_ioaddr + Cfg9346, Cfg9346_Lock);

	/* init Tx buffer DMA addresses */
	priv->tx_bufs_virt = (uint64_t)alloc_dma_coherent(TX_BUF_TOT_LEN, &priv->tx_buf_phys);
	for(i = 0; i < NUM_TX_DESC; i++){
		priv->tx_bufs[i] = priv->tx_bufs_virt + i * TX_BUF_SIZE; 
		outl(rt_ioaddr + TxAddr0 + (i * 4), priv->tx_buf_phys + i * TX_BUF_SIZE);
	}
	priv->cur_tx = 0;

	global_rtl_priv = priv;

	outl(rt_ioaddr + RxMissed, 0);
	//rtl8139_set_rx_mode(dev);

	/* no early-rx interrupts */
	outw(rt_ioaddr + MultiIntr, inw(rt_ioaddr + MultiIntr) & MultiIntrClear);
    
	/* make sure RxTx has started */
	tmp = inb(rt_ioaddr + ChipCmd);
	if((!(tmp & 0x08)) || (!(tmp & 0x04))){
		printk("\tRxTx not enabled, trying again...\n");
		outb(rt_ioaddr + 0x37, 0x0C);
	}

	/* set IMR + ISR
	 * The Interrupt Mask Register (IMR) and Interrupt Service Register (ISR) are responsible for firing up different IRQs.
	 * The IMR bits line up with the ISRS bits to work in sync.
	 * If an IMR bit is low, then the corresponding ISR bit will never fire an IRQ when the time comes for it to happen.
	 * The IMR is located at 0x3C and the ISR is located at 0x3E. */
	//To set the RTL8139 to accept only the Transmit OK (TOK) and Receive OK (ROK) interrupts, we would have the TOK and ROK
	//bits of the IMR high and leave the rest low.
	intr_mask = PCIErr | PCSTimeout | RxUnderrun | RxOverflow | RxFIFOOver | TxErr | TxOK | RxErr | RxOK;
	outw(rt_ioaddr + 0x3C, intr_mask);

	/* printk("before here\n"); */
	/* outw(rt_ioaddr + 0x3E, 0x1); */
	/* printk("after here\n"); */

	printk_info("\trtl8139 initialization complete!\n");
	printk_info("-------------------------------------------------------------\n");
	print_rtl8139_info(priv);

	//enable interrupts
	asm("STI");
	
	return 0;
}

// NOTE: CRC code taken from https://barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
typedef uint8_t crc;

#define POLYNOMIAL 0xD8
#define WIDTH (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))

//static crc crcTable[256];
//static int crc_initialized = 0;

/* static void crcInit(){ */
/* 	crc remainder; */
/* 	int dividend; */
/* 	uint8_t bit; */
	
/* 	/\* Compute the remainder of each possible dividend *\/ */
/* 	for(dividend = 0; dividend < 256; ++dividend){ */
/* 		/\* Start with the dividend followed by zeros. *\/ */
/* 		remainder = dividend << (WIDTH - 8); */

/* 		/\* Perform Modulo-2 division, a bit at a time. *\/ */
/* 		for(bit = 8; bit > 0; --bit){ */
/* 			if(remainder & TOPBIT){ */
/* 				remainder = (remainder << 1) ^ POLYNOMIAL; */
/* 			} */
/* 			else { */
/* 				remainder = (remainder << 1); */
/* 			} */
/* 		} */

/* 		/\* store the result in the crc table *\/ */
/* 		crcTable[dividend] = remainder; */
/* 	} */

/* 	crc_initialized = 1; */
/* } */

//todo: this should probably be moved somewhere else once I get transmit working
static int copy_data_with_checksum(uint8_t *data, uint64_t data_size, uint64_t tx_buf_beginning){
	uint8_t *tx_buf_cur = (uint8_t*)tx_buf_beginning;
	uint8_t cur_byte;
	//uint8_t modified_cur_byte;
	//crc remainder = 0;
	int i;
	
	if(data_size >= TX_BUF_SIZE){
		printk_err("attempted copy too large of an ethernet frame into transmit buffer, halting\n");
		asm("hlt");
		return -1;
	}

	/* if(!crc_initialized){ */
	/* 	crcInit(); */
	/* } */
	printk("***** SENDING ETH FRAME *****\n");
	/* Copy data byte by byte while generating crc */
	for(i = 0; i < data_size;){
		cur_byte = data[i++];
		/* modified_cur_byte = cur_byte ^ (remainder >> (WIDTH - 8)); */
		/* remainder = crcTable[modified_cur_byte] ^ (remainder << 8); */
		printk("%02hx ", cur_byte);
	        if(i % 16 == 0){
			printk("\n");
		}
		else if(i % 8 == 0){
			printk(" ");
		}
		*tx_buf_cur = cur_byte;
		tx_buf_cur += 1;
	}
	printk("\n****************************\n");

	return 0;
}

//https://www.cs.usfca.edu/~cruse/cs326f04/RTL8139_ProgrammersGuide.pdf
//The process of transmitting a packet:
int rtl8139_transmit_packet(uint8_t *data, uint64_t data_size){
	rt8139_private *priv = global_rtl_priv; //todo stop using globals
	uint8_t tx_buf_to_use;

	//disable interrupts
	asm("CLI");

	parse_eth_frame(data, data_size);
	
	if(priv == NULL){
		printk_err("rtl8139 seemingly not initialized, cannot transmit\n");
		return -1;
	}
	
        /* 1: copy the packet to a physically continuous buffer in memory */
	// calculate the next Tx descriptor entry
	tx_buf_to_use = priv->cur_tx % NUM_TX_DESC;

	// make sure the data is of correct length
	if(data_size < TX_BUF_SIZE) {
		//check if packet size is smaller than minimum # of frame octets
		if(data_size < ETH_ZLEN) {
			int i;
			uint8_t *payload = (uint8_t*)priv->tx_bufs[tx_buf_to_use];
			for(i = 0; i < ETH_ZLEN; i++){
				payload[i] = 0;
			}
		}

		//copy data while calculating checksum and append checksum to end
		copy_data_with_checksum(data, data_size, priv->tx_bufs[tx_buf_to_use]);
	}
	else {
		printk_err("attempted to transmit too large of an ethernet frame\n");
		asm("hlt");
	}
	
	/* 2: Write the descriptor which is functioning */
	//2.1: Fill in the Start Address (physical address) of this buffer.
	//this seems to only be done during initialization

	//2.2: Fill in Transmit Status: the size of this packet, the early transmit threshold, Clear OWN bit in TSD (this starts the PCI operation)
	outl(global_rt_ioaddr + TxStatus0 + (tx_buf_to_use * sizeof(uint32_t)),
	     ((TX_FIFO_THRESH << 11) & 0x003f0000) | (data_size > ETH_ZLEN ? data_size : ETH_ZLEN));

	priv->cur_tx += 1;

	printk_info("Queued Tx packet size %u to slot %d\n", data_size, tx_buf_to_use);
	
	//enable interrupts
	asm("STI");
	

	return 0;
}
