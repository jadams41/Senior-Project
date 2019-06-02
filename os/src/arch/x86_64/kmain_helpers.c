#include <stdint-gcc.h>
#include "drivers/block/blockDeviceDriver.h"
#include "drivers/fs/fat32.h"
#include "drivers/fs/vfs.h"
#include "drivers/interrupts/idt.h"
#include "drivers/memory/memoryManager.h"
#include "drivers/net/ethernet/realtek/8139too.h"
#include "drivers/pci/pci.h"
#include "drivers/ps2/keyboard.h"
#include "drivers/ps2/ps2Driver.h"
#include "drivers/serial/serial.h"
#include "net/arp/arp.h"
#include "net/ethernet/ethernet.h"
#include "net/ip/icmp.h"
#include "net/ip/ipv4.h"
#include "net/ip/udp.h"
#include "net/netdev.h"
#include "test/snakes/snakes.h"
#include "test/test.h"
#include "types/process.h"
#include "types/string.h"
#include "utils/byte_order.h"
#include "utils/parseMultiboot.h"
#include "utils/printk.h"
#include "utils/utils.h"

extern PROC_context *curProc;
extern struct BlockDev *ata;
extern struct PCIDevice *pci_devices;
extern net_device_list netdevs;

//takes params uint16_t base, uint16_t master, uint8_t slave, uint8_t irq
void initBlockDevice(void *params)
{
	uint64_t *paramArr;
	int arrLen;
	uint16_t base;
	uint16_t master;
	uint8_t slave;
	uint8_t irq;

	if (params == 0) {
		printk_err
		    ("did not pass in any parameters to init block device, nothing will happen\n");
		kexit();
	}
	paramArr = params;
	arrLen = *paramArr;
	if (arrLen - 1 != 4) {
		printk_err
		    ("[initBlockDevice]: passed in the wrong number of parameters, expected 4 and received %d\n",
		     arrLen - 1);
		kexit();
	}

	base = paramArr[1];
	master = paramArr[2];
	slave = paramArr[3];
	irq = paramArr[4];

	ata = ata_probe(base, master, slave, irq);
	kexit();
}

int init_pci_devices()
{
	int num_devices_initd = 0;
	PCIDevice *cur = pci_devices;

	net_device *new_netdev = NULL;
	
	while (cur) {
		switch (cur->vendor_id) {
		case 0x10ec:
			switch (cur->device_id) {
			case 0x8139:
			        new_netdev = init_rt8139(cur);
				if(new_netdev){
					print_dev_addrs(new_netdev);
					add_net_dev(netdevs, new_netdev);
				}
				break;
			}

			break;
		}
		cur = cur->next_device;
	}

	return num_devices_initd;
}
