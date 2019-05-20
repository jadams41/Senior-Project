#include <stdint-gcc.h>
#include "drivers/net/8139too.h"
#include "drivers/pci/pci.h"
#include "drivers/memory/memoryManager.h"
#include "drivers/interrupts/idt.h"
#include "utils/printk.h"
#include "utils/utils.h"
#include "utils/byte_order.h"

/* RealTek RTL-8139 Fast Ethernet driver */

/* Symbolic offsets to registers. (taken from linux driver) */
enum RTL8139_registers {
    MAC0              = 0,	/* Ethernet hardware address. */
    MAR0              = 8,	/* Multicast filter. */
    TxStatus0         = 0x10,	/* Transmit status (Four 32bit registers). */
    TxAddr0           = 0x20,	/* Tx descriptors (also four 32bit). */
    RxBuf             = 0x30,
    ChipCmd           = 0x37,
    RxBufPtr          = 0x38,
    RxBufAddr         = 0x3A,
    IntrMask          = 0x3C,
    IntrStatus        = 0x3E,
    TxConfig          = 0x40,
    RxConfig          = 0x44,
    Timer             = 0x48,	/* A general-purpose counter. */
    RxMissed          = 0x4C,	/* 24 bits valid, write clears. */
    Cfg9346           = 0x50,
    Config0           = 0x51,
    Config1           = 0x52,
    FlashReg          = 0x54,
    MediaStatus       = 0x58,
    Config3           = 0x59,
    Config4           = 0x5A,	/* absent on RTL-8139A */
    HltClk            = 0x5B,
    MultiIntr         = 0x5C,
    TxSummary         = 0x60,
    BasicModeCtrl     = 0x62,
    BasicModeStatus   = 0x64,
    NWayAdvert        = 0x66,
    NWayLPAR          = 0x68,
    NWayExpansion     = 0x6A,
    
    /* Undocumented registers, but required for proper operation. */
    FIFOTMS           = 0x70,	/* FIFO Control and test. */
    CSCR              = 0x74,	/* Chip Status and Configuration Register. */
    PARA78            = 0x78,
    PARA7c            = 0x7c,	/* Magic transceiver parameter register. */
    Config5           = 0xD8,	/* absent on RTL-8139A */
};

enum ClearBitMasks {
    MultiIntrClear	= 0xF000,
    ChipCmdClear	= 0xE2,
    Config1Clear	= (1<<7)|(1<<6)|(1<<3)|(1<<2)|(1<<1),
};

enum ChipCmdBits {
    CmdReset	= 0x10,
    CmdRxEnb	= 0x08,
    CmdTxEnb	= 0x04,
    RxBufEmpty	= 0x01,
};

enum IntrStatusBits {
    PCIErr     	= 0x8000,
    PCSTimeout	= 0x4000,
    RxFIFOOver	= 0x40,
    RxUnderrun	= 0x20,
    RxOverflow	= 0x10,
    TxErr	= 0x08,
    TxOK	= 0x04,
    RxErr	= 0x02,
    RxOK	= 0x01,
    
    RxAckBits	= RxFIFOOver | RxOverflow | RxOK,
};

enum TxStatusBits {
    TxHostOwns	= 0x2000,
    TxUnderrun	= 0x4000,
    TxStatOK	= 0x8000,
    TxOutOfWindow	= 0x20000000,
    TxAborted	= 0x40000000,
    TxCarrierLost	= 0x80000000,
};

/* bit flags in the Rx Status field found the Rx Packet header
 * NOTE: Rx Status field is consulted when an Rx interrupt is triggered
 *       and the packet header and packet are placed in the Rx Ring Buffer */
enum RxStatusBits {
    RxMulticast	= 0x8000, /* MAR - Multicast Address received: Set to 1 indicates 
			     that multicast packet is received */ 

    RxPhysical	= 0x4000, /* PAM - Physical Address Matched: Set to 1 indicates that
			     the destination address of this packet matches 
			     matches the value written in the ID registers (MAC) */

    RxBroadcast	= 0x2000, /* BAR - Broadcast Address Received: Set to 1 indicates
			     that a broadcast packet is received. BAR, MAR will
			     not be set simultaneously. */
    
    RxBadSymbol	= 0x0020, /* ISE - Invalid Symbol Error: (100Base-TX only) An 
			     invalid symbol was encountered during the reception
			     of this packet if this bit is set to 1. */
    
    RxRunt	= 0x0010, /* RUNT - Runt packet received: Set to 1 indicates
			     that the received packet length is smaller than 64
			     bytes */
    
    RxTooLong	= 0x0008, /* LONG - Long Packet: Set to 1 indicates that the
			     size of the received packet exceeds 4k bytes */
    
    RxCRCErr	= 0x0004, /* CRC - CRC Error: When set, indicates that a CRC
			     error occurred on the received packet. */
    
    RxBadAlign	= 0x0002, /* FAE - Frame Alignment Error: When set, indicates
			     that a frame alignmnent error occurred on this
			     received packet. */

    RxStatusOK	= 0x0001, /* ROK - Receive OK: When set, indicates that a good
			     packet is received */
};

enum RxConfigBits {
	/* rx fifo threshold */
	RxCfgFIFOShift	= 13,
	RxCfgFIFONone	= (7 << RxCfgFIFOShift),

	/* Max DMA burst */
	RxCfgDMAShift	= 8,
	RxCfgDMAUnlimited = (7 << RxCfgDMAShift),

	/* rx ring buffer length */
	RxCfgRcv8K	= 0,
	RxCfgRcv16K	= (1 << 11),
	RxCfgRcv32K	= (1 << 12),
	RxCfgRcv64K	= (1 << 11) | (1 << 12),

	/* Disable packet wrap at end of Rx buffer. (not possible with 64k) */
	RxNoWrap	= (1 << 7),
};

enum tx_config_bits {
    /* Interframe Gap Time. Only TxIFG96 doesn't violate IEEE 802.3 */
    TxIFGShift	= 24,
    TxIFG84		= (0 << TxIFGShift), /* 8.4us / 840ns (10 / 100Mbps) */
    TxIFG88		= (1 << TxIFGShift), /* 8.8us / 880ns (10 / 100Mbps) */
    TxIFG92		= (2 << TxIFGShift), /* 9.2us / 920ns (10 / 100Mbps) */
    TxIFG96		= (3 << TxIFGShift), /* 9.6us / 960ns (10 / 100Mbps) */
    
    TxLoopBack	= (1 << 18) | (1 << 17), /* enable loopback test mode */
    TxCRC		= (1 << 16),	/* DISABLE Tx pkt CRC append */
    TxClearAbt	= (1 << 0),	/* Clear abort (WO) */
    TxDMAShift	= 8, /* DMA burst value (0-7) is shifted X many bits */
    TxRetryShift	= 4, /* TXRR value (0-15) is shifted X many bits */
    
    TxVersionMask	= 0x7C800000, /* mask out version bits 30-26, 23 */
};

enum Config1Bits {
    Cfg1_PM_Enable	= 0x01,
    Cfg1_VPD_Enable	= 0x02,
    Cfg1_PIO	        = 0x04,
    Cfg1_MMIO	        = 0x08,
    LWAKE		= 0x10,		/* not on 8139, 8139A */
    Cfg1_Driver_Load    = 0x20,
    Cfg1_LED0	        = 0x40,
    Cfg1_LED1	        = 0x80,
    SLEEP		= (1 << 1),	/* only on 8139, 8139A */
    PWRDN		= (1 << 0),	/* only on 8139, 8139A */
};

enum Config4Bits {
    LWPTN = (1 << 2),	/* not on 8139, 8139A */
};

enum Cfg9346Bits {
    Cfg9346_Lock	= 0x00,
    Cfg9346_Unlock	= 0xC0,
};

enum Ethertypes {
    ETHRTYPE_IPV4 = 0x0800,
    ETHRTYPE_ARP  = 0x0806,
    ETHRTYPE_IPV6 = 0x86DD,
};

static const unsigned int rtl8139_tx_config =
    TxIFG96 | (TX_DMA_BURST << TxDMAShift) | (TX_RETRY << TxRetryShift);

void print_mac_addr(uint8_t *mac){
    printk("%02X:%02X:%02X:%02X:%02X:%02X",
	   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

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

//todo clean this up, just using to test functionality inside of isr
uint32_t global_rt_ioaddr = 0;
uint64_t global_rx_buffer_virt = 0;
uint64_t global_rx_buffer_size = 0;
rt8139_private *global_rtl_priv = NULL;

void parse_eth_frame(uint8_t *eth_frame, unsigned int frame_size){
    uint8_t *dest_mac = eth_frame;
    uint8_t *source_mac = eth_frame + 6;
    uint16_t ethertype_or_len = ntohs(*((uint16_t*)(eth_frame + 12)));
    
    printk("****** Received Ethernet Frame ******\n");
    printk("Total Length: %u\n", frame_size);
    
    printk("Dest   MAC: ");
    print_mac_addr(dest_mac);
    printk("\n");

    printk("Source MAC: ");
    print_mac_addr(source_mac);
    printk("\n");

    if(ethertype_or_len < 1501){
    	printk("Data Length: %hu\n", ethertype_or_len);
    }
    else {
    	printk("Ethertype: ");
    	print_ethertype(ethertype_or_len);
    	printk(" (0x%04x)\n", ethertype_or_len);
    }
    
    printk("*************************************\n\n");
}

/* Inform rtl8139 that packet was successfully received 
 * NOTE: acknowledgement procedure taken from Linux Source
 */
static void rtl8139_rx_isr_ack(uint32_t rt_ioaddr){
    static uint64_t rx_errors = 0, rx_fifo_errors = 0;
    uint16_t status;

    status = inw(rt_ioaddr + IntrStatus) & RxAckBits;

    /* clear out errors and receive interrupts */
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

int rtl_rx(){
    if(global_rtl_priv == NULL){
	printk_err("rtl not initialized, can't receive packet\n");
	asm("hlt");
    }

    int received = 0;

    uint32_t rt_ioaddr = global_rt_ioaddr;
    
    rt8139_private *priv = global_rtl_priv;
    uint8_t *rx_buf = (uint8_t*)priv->rx_buf_virt;
    uint64_t cur_rx = priv->cur_rx;
    uint64_t rx_buf_size = priv->rx_buf_size;
    
    unsigned int rx_size = 0;
    
    //loop until rx buffer is empty
    while((inb(rt_ioaddr + ChipCmd) & RxBufEmpty) == 0){
	uint32_t ring_offset = cur_rx % rx_buf_size;
	uint32_t rx_status;
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

	/* todo: seems to be a problem when ring buffer wraps */
	//move to next packet (logic taken from linux source, assuming that this properly aligns cur_rx to the next packet)
	cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
	outw(rt_ioaddr + RxBufPtr, (uint16_t) (cur_rx - 16));
	
	uint8_t *eth_frame_beginning = (uint8_t*)(rx_buf + ring_offset + 4);
	parse_eth_frame(eth_frame_beginning, rx_size);
	
	//acknowledge packet
	rtl8139_rx_isr_ack(rt_ioaddr);
    }

    //update device's private cur_rx
    priv->cur_rx = cur_rx;

 out:
    return received;
}

void rtl_isr(int irq, int err){
    //printk("received rtl interrupt\n", irq);

    int received = rtl_rx();
    printk_info("received %d packets\n", received);
}

void rtl8139_wake_up(uint32_t rt_ioaddr){
    uint8_t tmp8, new_tmp8;
    new_tmp8 = tmp8 = inb(rt_ioaddr + Config1);
    new_tmp8 |= Cfg1_PM_Enable;

    if(/*HasLWake && */new_tmp8 & LWAKE){
	printk("setting lwake\n");
	new_tmp8 &= ~LWAKE;
    }
    else {
	printk("ignoring LWAKE\n");
    }
    
    if(new_tmp8 != tmp8){
	printk("setting new config1\n");
	outb(rt_ioaddr + Cfg9346, Cfg9346_Unlock);
	outb(rt_ioaddr + Config1, tmp8);
	outb(rt_ioaddr + Cfg9346, Cfg9346_Lock);    
    }
    else {
	printk("new_tmp8 same as old tmp8\n");
    }
    /*if(HasLWake && )*/
    tmp8 = inb(rt_ioaddr + Config4);
    if(tmp8 & LWPTN){
	printk("setting new config4\n");
	outb(rt_ioaddr + Cfg9346, Cfg9346_Unlock);
	outb(rt_ioaddr + Config1, tmp8 & ~LWPTN);
	outb(rt_ioaddr + Cfg9346, Cfg9346_Lock);
    }
}

void rtl8139_chip_reset(uint32_t rt_ioaddr){
    // send 0x10 to Command register
    printk_info("initializing software reset...");
    outb(rt_ioaddr + ChipCmd, CmdReset);

    // poll RST bit until not set to 1
    uint8_t cmd_reg;
    while(((cmd_reg = inb(rt_ioaddr + ChipCmd)) & CmdReset) != 0)
	printk("cmd_reg = 0x%x\n", cmd_reg);
    
    printk(" done!\n");
}

int init_rt8139(PCIDevice *dev) {
    rt8139_private *priv = (rt8139_private*)kmalloc(sizeof(rt8139_private));
    priv->dev = dev;

    printk("rtl8139 uses interrupt line 0x%x\n", dev->interrupt_line);
    IRQ_set_handler(dev->interrupt_line + 0x20, rtl_isr);
    IRQ_clear_mask(dev->interrupt_line);

    
    /* Enable PCI Bus Mastering */
    // read command register from the device's PCI Configuration space, set bit 2 (bus mastering bit), and write the modified command register
    printk_info("enabling bus mastering for rtl839\n");
    uint32_t command = pci_config_read_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, 2);
    command |= 0b100; //enable bus mastering
    command |= 0b1; //enable I/O space
    command |= 0b10; //enable Memory space
    command |= 0b10000; //enable memory write
    //command &= 0b01111111111; //disable interrupt disable

    if(command & 0b10000000000){
	printk_info("interrupts disabled\n");
    }
    else {
	printk_info("interrupts enabled\n");
    }
    
    pci_config_write_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, command, 2);

    if(!(dev->io_addrs)){
	printk_err("supplied rt8139 has no base i/o address, can't initialize\n");
	return 1;
    }
    
    uint32_t rt_ioaddr = global_rt_ioaddr = dev->io_addrs->base_addr;
    printk_info("rtl8139 using ioaddr: 0x%x\n", rt_ioaddr);

    /* turn on the device */
    //bring old chips out of low power mode
    outb(rt_ioaddr + HltClk, 'R');
    
    printk_info("turning on rtl8139\n");
    rtl8139_wake_up(rt_ioaddr);
    
    /* software reset */
    rtl8139_chip_reset(rt_ioaddr);

    uint64_t mac_addr = 0;
    mac_addr += le32_to_cpu(inl(rt_ioaddr + MAC0));
    mac_addr <<= 16;
    mac_addr += le32_to_cpu(inl(rt_ioaddr + MAC0 + 4));
    printk("RTL8139 MAC ADDR: ");
    print_mac_addr(((uint8_t*)&mac_addr));
    printk("\n");

    /* unlock Config[01234] and BMCR register writes */
    outb(rt_ioaddr + Cfg9346, Cfg9346_Unlock);
    
    /* initialize chip's receive buffer */
    // size of 8192 + 16 (8K + 16 bytes) is recommended
    uint64_t rx_buffer_virt;
    uint32_t rx_buffer_phys;
    global_rx_buffer_virt = rx_buffer_virt = (uint64_t)alloc_dma_coherent(RX_BUF_TOT_LEN, &rx_buffer_phys);
    wipe_dma_buffer(rx_buffer_virt, 8192 + 16);
    
    outl(rt_ioaddr + RxBuf, rx_buffer_phys);

    global_rx_buffer_size = 8192;
    
    priv->rx_buf_virt = rx_buffer_virt;
    priv->rx_buf_phys = rx_buffer_phys;
    priv->rx_buf_size = 8192;
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
    uint32_t tx_buf_phys;
    priv->tx_bufs = (unsigned char*)alloc_dma_coherent(TX_BUF_TOT_LEN, &tx_buf_phys);
    int i;
    for(i = 0; i < NUM_TX_DESC; i++){
	priv->tx_buf[i] = &(priv->tx_bufs[i * TX_BUF_SIZE]); 
	outl(rt_ioaddr + TxAddr0 + (i * 4), tx_buf_phys + (priv->tx_buf[i] - priv->tx_bufs));
    }

    global_rtl_priv = priv;

    outl(rt_ioaddr + RxMissed, 0);
    //rtl8139_set_rx_mode(dev);

    /* no early-rx interrupts */
    outw(rt_ioaddr + MultiIntr, inw(rt_ioaddr + MultiIntr) & MultiIntrClear);
    
    /* make sure RxTx has started */
    uint8_t tmp = inb(rt_ioaddr + ChipCmd);
    if((!(tmp & 0x08)) || (!(tmp & 0x04))){
	printk("RxTx not enabled, trying again\n");
	outb(rt_ioaddr + 0x37, 0x0C);
    }

    /* set IMR + ISR
     * The Interrupt Mask Register (IMR) and Interrupt Service Register (ISR) are responsible for firing up different IRQs.
     * The IMR bits line up with the ISRS bits to work in sync.
     * If an IMR bit is low, then the corresponding ISR bit will never fire an IRQ when the time comes for it to happen.
     * The IMR is located at 0x3C and the ISR is located at 0x3E. */
    //To set the RTL8139 to accept only the Transmit OK (TOK) and Receive OK (ROK) interrupts, we would have the TOK and ROK
    //bits of the IMR high and leave the rest low.
    uint16_t intr_mask = PCIErr | PCSTimeout | RxUnderrun | RxOverflow | RxFIFOOver | TxErr | TxOK | RxErr | RxOK;
    outw(rt_ioaddr + 0x3C, intr_mask);

    /* printk("before here\n"); */
    /* outw(rt_ioaddr + 0x3E, 0x1); */
    /* printk("after here\n"); */

    return 0;
}

int rtl8139_transmit_packet(){
    return 0;
}
