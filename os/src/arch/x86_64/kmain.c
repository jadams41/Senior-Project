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
#include "net/ip/tcp.h"
#include "net/ip/udp.h"
#include "net/netdev.h"
#include "test/snakes/snakes.h"
#include "test/test.h"
#include "test/test_functions.h"
#include "types/process.h"
#include "types/string.h"
#include "utils/byte_order.h"
#include "utils/parseMultiboot.h"
#include "utils/printk.h"
#include "utils/utils.h"

/* assembly routine wrappers */
extern void perform_syscall(int);
extern void load_page_table(uint64_t);
extern void store_control_registers();
/* assembly variables */
extern uint64_t saved_cr2;
extern uint64_t saved_cr3;
extern uint64_t vga_buf_cur;

/* kmain helper functions */
extern void initBlockDevice(void *params);
extern int init_pci_devices();


int stupidFunctionDead = 0;

/*** KERNEL STATE VARIABLES ***/
extern PROC_context *curProc;
struct BlockDev *ata = 0;

/* list of all discovered pci devices */
struct PCIDevice *pci_devices = NULL;

/* list of all discovered network devices */
net_device_list netdevs = {
	.count = 0,
	.list = {
		.first = NULL,
		.last = NULL
	},
};

// configuration globals
int no_debug = 0;
enum printing_level {
	info,			//print everything
	warn,			//exclude info
	err,			//exclude info and warn
	silent
} printk_conf = info;


int kmain(void *multiboot_point, unsigned int multitest)
{
	TagStructureInfo *tagStructureInfo;
	GenericTagHeader *curTag;
	uint64_t ****new_page_table;
	int num_pci_devs;
	int enabled = 0;
	ipv4_addr static_ip;

	asm("cli");

	//set up and test the VGA
	VGA_clear();
	// vgaDispCharTest();
	// VGA_display_str("string print test\n");

	//set up the things needed for printk
	initialize_scancodes();
	initialize_shift_down_dict();
	disableSerialPrinting();

	//currently working without this, too scared to uncomment
	// initPs2();
	// keyboard_config();

	//initialize interrupts
	idt_init();
	IRQ_clear_mask(KEYBOARD_IRQ);
	IRQ_clear_mask(SERIAL_COM1_IRQ);
	IRQ_clear_mask(SLAVE_PIC_IRQ);
	IRQ_clear_mask(PRIMARY_ATA_CHANNEL_IRQ);
	// IRQ_clear_mask(SECONDARY_ATA_CHANNEL_IRQ);

	init_kbd_state();

	//turn on interrupts
	asm("sti");

	//initialize serial output
	SER_init();
	printk_rainbow("----SERIAL DEBUGGING BEGIN----\n");

	printk_info("%s\n",
		    get_endianness() == LITTLE_ENDIAN ?
		    "Endianness: Little Endian" : "Endianness: Big Endian");

	//initialize the memory info datastructure
	init_usable_segment_struct();

	//take the multiboot_point from entering long mode, and figure out where tags BEGIN
	tagStructureInfo = (TagStructureInfo *) multiboot_point;
	printk("tagStructInfo: total_size=%u reserved=%u\n",
	       tagStructureInfo->totalBytes, tagStructureInfo->reserved);
	curTag = (GenericTagHeader *) ((uint64_t) tagStructureInfo + 8);

	//iterate over and parse all multiboot tags
	while (curTag) {
		printTagInfo(curTag);
		potentiallyUseTag(curTag);
		curTag = getNextTag(curTag);
	}

	store_control_registers();
	new_page_table = init_page_table();
	load_page_table((uint64_t) new_page_table);

	init_kheap();
	ata = ata_probe(0x1F0, 0x40, 0, 0x14);

	printk_info("Searching for connected PCI devices:\n");
	num_pci_devs = pci_probe(&pci_devices);
	printk_info("Found %d pci devices\n", num_pci_devs);

#ifdef STATIC_IP
	if(!(static_ip = str_to_ipv4(STATIC_IP))){
		printk_err("defined static ip (`%s`) is invalid\n", STATIC_IP);
		asm("hlt");
	}
	
	if(netdevs.count > 0){
		net_device_node *first = (net_device_node*)(netdevs.list.first);
		add_ipv4_addr(first->dev, static_ip);
	}
#endif
	
	if (num_pci_devs) {
		init_pci_devices();
	}

	init_tcp_state();
	
	while (!enabled) ;
	//send_test_udp();
        //send_ping();
	//test_tcp_syn();
	tcp_connect(str_to_ipv4(STATIC_IP), str_to_ipv4("172.16.210.183"), 57746, 2023);
	
	while (1) {
		PROC_run();
		asm("hlt");
	}

	return 0;
}
