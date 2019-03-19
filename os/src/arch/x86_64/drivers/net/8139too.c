#include <stdint-gcc.h>
#include "drivers/net/8139too.h"
#include "drivers/pci/pci.h"
#include "drivers/memory/memoryManager.h"
#include "utils/printk.h"
#include "utils/utils.h"

/* RealTek RTL-8139 Fast Ethernet driver */

/* Symbolic offsets to registers. (taken from linux driver) */
enum RTL8139_registers {
	MAC0 = 0,		/* Ethernet hardware address. */
	MAR0 = 8,		/* Multicast filter. */
	TxStatus0 = 0x10,	/* Transmit status (Four 32bit registers). */
	TxAddr0 = 0x20,		/* Tx descriptors (also four 32bit). */
	RxBuf = 0x30,
	ChipCmd = 0x37,
	RxBufPtr = 0x38,
	RxBufAddr = 0x3A,
	IntrMask = 0x3C,
	IntrStatus = 0x3E,
	TxConfig = 0x40,
	RxConfig = 0x44,
	Timer = 0x48,		/* A general-purpose counter. */
	RxMissed = 0x4C,	/* 24 bits valid, write clears. */
	Cfg9346 = 0x50,
	Config0 = 0x51,
	Config1 = 0x52,
	FlashReg = 0x54,
	MediaStatus = 0x58,
	Config3 = 0x59,
	Config4 = 0x5A,		/* absent on RTL-8139A */
	HltClk = 0x5B,
	MultiIntr = 0x5C,
	TxSummary = 0x60,
	BasicModeCtrl = 0x62,
	BasicModeStatus = 0x64,
	NWayAdvert = 0x66,
	NWayLPAR = 0x68,
	NWayExpansion = 0x6A,
	/* Undocumented registers, but required for proper operation. */
	FIFOTMS = 0x70,		/* FIFO Control and test. */
	CSCR = 0x74,		/* Chip Status and Configuration Register. */
	PARA78 = 0x78,
	PARA7c = 0x7c,		/* Magic transceiver parameter register. */
	Config5 = 0xD8,		/* absent on RTL-8139A */
};

int init_rt8139(PCIDevice *dev) {
    rt8139_private *priv = (rt8139_private*)kmalloc(sizeof(rt8139_private));
    priv->dev = dev;
    
    /* Enable PCI Bus Mastering */
    // read command register from the device's PCI Configuration space, set bit 2 (bus mastering bit), and write the modified command register
    printk_info("enabling bus mastering for rtl839\n");
    uint32_t command = pci_config_read_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, 2);
    command |= 0b100;
    pci_config_write_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, command, 2);

    if(!(dev->io_addrs)){
	printk_err("supplied rt8139 has no base i/o address, can't initialize\n");
	return 1;
    }
    
    uint32_t rt_ioaddr = dev->io_addrs->base_addr;
    printk_info("rtl8139 using ioaddr: 0x%x\n", rt_ioaddr);
    /* turn on the device */
    // send 0x00 to the CONFIG_1 register (0x52) to set the LWAKE + LWPTN to active high.
    printk_info("turning on rtl8139\n");
    outb(0x0, rt_ioaddr + 0x52);

    /* software reset */
    // send 0x10 to Command register
    printk_info("initializing software reset...");
    outb(0x10, rt_ioaddr + 0x37);

    // poll RST bit until not set to 1
    uint8_t cmd_reg;
    while(((cmd_reg = inb(rt_ioaddr) + 0x37) & 0x10) != 0)
	printk("cmd_reg = 0x%x\n", cmd_reg);
    
    printk(" done!\n");
    
    /* initialize chip's receive buffer */
    // size of 8192 + 16 (8K + 16 byttes) is recommended
    uint64_t rx_buffer_virt;
    uint32_t rx_buffer_phys;
    rx_buffer_virt = (uint64_t)alloc_dma_coherent(8192 + 16, &rx_buffer_phys);
    outl(rx_buffer_phys, rt_ioaddr + 0x30);

    priv->rx_buf_virt = rx_buffer_virt;
    priv->rx_buf_phys = rx_buffer_phys;

    /* set IMR + ISR
     * The Interrupt Mask Register (IMR) and Interrupt Service Register (ISR) are responsible for firing up different IRQs.
     * The IMR bits line up with the ISRS bits to work in sync.
     * If an IMR bit is low, then the corresponding ISR bit will never fire an IRQ when the time comes for it to happen.
     * The IMR is located at 0x3C and the ISR is located at 0x3E. */
    //To set the RTL8139 to accept only the Transmit OK (TOK) and Receive OK (ROK) interrupts, we would have the TOK and ROK
    //bits of the IMR high and leave the rest low.
    outw(0x0005, rt_ioaddr + 0x3C);
    
    /* configure the receive buffer */
    outl(0xf | (1 << 7), rt_ioaddr + 0x44);

    /* enable receive and transmitter
     * Start up the RX and TX functions.
     * The RE (Receiver Enabled) and TE (Transmitter Enabled) bits are located in the command register (0x37). */
    outb(0x0C, rt_ioaddr + 0x37);

    
    return 0;
}
