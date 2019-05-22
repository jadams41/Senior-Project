#ifndef _8139TOO
#define _8139TOO

#include "drivers/pci/pci.h"

#define NUM_TX_DESC 4
#define MAX_ETH_FRAME_SIZE 1792

#define RX_BUF_LEN 8192
#define RX_BUF_PAD 16
#define RX_BUF_WRAP_PAD 2048 /* spare padding to handle lack of packet wrap */
#define RX_BUF_TOT_LEN (RX_BUF_LEN + RX_BUF_PAD + RX_BUF_WRAP_PAD)

#define TX_BUF_SIZE MAX_ETH_FRAME_SIZE
#define TX_BUF_TOT_LEN (TX_BUF_SIZE * NUM_TX_DESC)

#define RX_FIFO_THRESH	7	/* Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST	7	/* Maximum PCI burst, '6' is 1024 */
#define TX_DMA_BURST	6	/* Maximum PCI burst, '6' is 1024 */
#define TX_RETRY	8	/* 0-15.  retries = 16 + (TX_RETRY * 16) */

/* private device configuration information structure */
typedef struct rt8139_private {
	PCIDevice *dev;

	/* miscellaneous information */
        uint64_t mac_addr; //NOTE: the mac address is only going to be six bytes

	/* rx ring-buffer information */
	uint64_t rx_buf_virt;       // virtual memory address of rx buffer (used by kernel)
	uint32_t rx_buf_phys;       // physical memory address of rx buffer (used by rtl8139 when performing dma)
	uint64_t rx_buf_size;       // size of the allocated rx buffer (without padding)
	uint64_t rx_buf_total_size; // size of the allocated rx buffer (with padding)
	uint64_t cur_rx;            // keeps track of current position inside rx ring buffer

	/* tx buffer(s) information */
	unsigned char *tx_buf[NUM_TX_DESC];
	unsigned char *tx_bufs;             //beginning virtual address of the tx buffers
	uint32_t tx_buf_phys;               //physical address for the beginning of the tx buffer
} rt8139_private;

/* externally accessible functions */
int init_rt8139(PCIDevice *dev);


/* rtl8139 register offsets */
// Symbolic offsets to registers. (taken from linux driver)
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
	TxErr	        = 0x08,
	TxOK	        = 0x04,
	RxErr	        = 0x02,
	RxOK	        = 0x01,
    
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
    
	RxRunt	        = 0x0010, /* RUNT - Runt packet received: Set to 1 indicates
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
	RxCfgFIFOShift	  = 13,
	RxCfgFIFONone	  = (7 << RxCfgFIFOShift),

	/* Max DMA burst */
	RxCfgDMAShift	  = 8,
	RxCfgDMAUnlimited = (7 << RxCfgDMAShift),

	/* rx ring buffer length */
	RxCfgRcv8K	  = 0,
	RxCfgRcv16K	  = (1 << 11),
	RxCfgRcv32K	  = (1 << 12),
	RxCfgRcv64K	  = (1 << 11) | (1 << 12),

	/* Disable packet wrap at end of Rx buffer. (not possible with 64k) */
	RxNoWrap	  = (1 << 7),
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
	Cfg1_PM_Enable	        = 0x01,
	Cfg1_VPD_Enable	        = 0x02,
	Cfg1_PIO	        = 0x04,
	Cfg1_MMIO	        = 0x08,
	LWAKE		        = 0x10,	        /* not on 8139, 8139A */
	Cfg1_Driver_Load        = 0x20,
	Cfg1_LED0	        = 0x40,
	Cfg1_LED1	        = 0x80,
	SLEEP		        = (1 << 1),	/* only on 8139, 8139A */
	PWRDN		        = (1 << 0),	/* only on 8139, 8139A */
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

#endif
