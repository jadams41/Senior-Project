#include <stdint-gcc.h>
#include "drivers/block/blockDeviceDriver.h"
#include "drivers/fs/fat32.h"
#include "drivers/fs/vfs.h"
#include "drivers/interrupts/idt.h"
#include "drivers/net/8139too.h"
#include "drivers/net/arp/arp.h"
#include "drivers/net/ethernet/ethernet.h"
#include "drivers/memory/memoryManager.h"
#include "drivers/pci/pci.h"
#include "drivers/ps2/ps2Driver.h"
#include "drivers/ps2/keyboard.h"
#include "drivers/serial/serial.h"
#include "test/test.h"
#include "test/snakes/snakes.h"
#include "types/process.h"
#include "types/string.h"
#include "utils/byte_order.h"
#include "utils/parseMultiboot.h"
#include "utils/printk.h"
#include "utils/utils.h"

extern void perform_syscall(int);
extern void load_page_table(uint64_t);
extern void store_control_registers();
extern uint64_t saved_cr2;
extern uint64_t saved_cr3;
extern uint64_t vga_buf_cur;

int stupidFunctionDead = 0;
extern PROC_context *curProc;
struct BlockDev *ata = 0;
struct PCIDevice *pci_devices = NULL;

// configuration globals
int no_debug = 0;
enum printing_level {
	info,			//print everything
	warn,			//exclude info
	err,			//exclude info and warn
	silent
} printk_conf = info;

//takes params void *blockBuffer, uint64_t blockNumber
void printBlock(void *params)
{
	uint64_t *paramArr;
	uint64_t paramLen;
	uint8_t *block;
	uint64_t blockNum;
	uint64_t blockOffset;
	int i;
	int j;

	paramArr = (uint64_t *) params;
	if (params == 0) {
		printk_err
		    ("No parameters were passed to printBlock, nothing will happen\n");
		kexit();
	}

	paramLen = *paramArr;
	if (paramLen != 3) {
		printk_err
		    ("the wrong number of parameters were passed to printBlock, nothing will happen\n");
		kexit();
	}
	block = (uint8_t *) paramArr[1];
	blockNum = paramArr[2];
	blockOffset = blockNum * 512;

	/* printk("---------- BLOCK-%d ----------\n", blockNum); */
	for (i = 0; i < 512; i += 16) {
		printk("%06x ", blockOffset + i);
		for (j = 0; j < 16; j += 2) {
			printk("%02hx%02hx ", block[i + j + 1], block[i + j]);
		}
		printk("\n");
	}
	/* printk("---------------------------\n"); */
	kfree(params);
	kexit();
}

//takes params uint64_t blockNum, void *dst
void readBlock(void *params)
{
	uint64_t blockNum;
	void *dst;
	uint64_t *paramArr;
	uint64_t paramLen;
	uint64_t *printBlockParms;

	if (params == 0) {
		printk_err
		    ("No parameters were passed to readBlock, nothing will happen\n");
		kexit();
	}
	if (ata == 0) {
		printk_err("ata not initialized, nothing will happen\n");
		kexit();
	}

	paramArr = (uint64_t *) params;
	paramLen = *paramArr;
	if (paramLen - 1 != 2) {
		printk_err
		    ("passed the wrong number of parameters, expected 2 and received %llu",
		     paramLen - 1);
		kexit();
	}

	blockNum = paramArr[1];
	dst = (void *)paramArr[2];
	ata->read_block(ata, blockNum, dst);

	/* printk_info("done reading block %d, creating process to print this block now\n", blockNum); */

	printBlockParms = kmalloc(sizeof(uint64_t) * 3);
	printBlockParms[0] = 3;
	printBlockParms[1] = (uint64_t) dst;
	printBlockParms[2] = (uint64_t) blockNum;

	asm("CLI");
	PROC_create_kthread(printBlock, printBlockParms);
	asm("STI");

	kexit();
}

void grabKeyboardInput()
{
	printk("keyboard listening process is go (PID=%d)\n", curProc->pid);
	while (1) {
		printk("%c", KBD_read());
	}
}

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

/* void readBlock0(){ */
/*     char dest[512], dest2[512]; */
/*     memset(dest, 0, 512); */
/*     ata->read_block(ata, 2048, dest); */

/*     ExtendedFatBootRecord *efbr = (ExtendedFatBootRecord*)dest; */

/*     readBPB(&(efbr->bpb)); */

/*     unsigned int root_dir_sector = get_root_directory_sector((ExtendedFatBootRecord*)dest); */
/*     printk("root_dir_sector = %u\n", root_dir_sector); */

/*     uint32_t fat_size = efbr->sectors_per_fat; */
/*     uint64_t root_dir_sectors = 0; //0 on FAT32 */
/*     uint64_t first_data_sector = efbr->bpb.num_reserved_sectors + (efbr->bpb.num_fats * fat_size) + root_dir_sectors; */
/*     //uint64_t first_fat_sector = efbr->bpb.num_reserved_sectors; */
/*     printk("first data sector = %lu\n", first_data_sector + 2048); */

/*     // read in the root dir block */
/*     ata->read_block(ata,  first_data_sector + 2048, dest2); */
/*     // parse entries in the root dir */
/*     printk("--------- ROOT DIRECTORY ---------\n"); */
/*     uint8_t *entry = (uint8_t*)dest2; */
/*     read_directory(entry); */
/*     kexit(); */
/* } */

int init_pci_devices()
{
	int num_devices_initd = 0;
	PCIDevice *cur = pci_devices;

	while (cur) {
		switch (cur->vendor_id) {
		case 0x10ec:
			switch (cur->device_id) {
			case 0x8139:
				if (!init_rt8139(cur)) {
					num_devices_initd += 1;
				}
				break;
			}

			break;
		}
		cur = cur->next_device;
	}

	return num_devices_initd;
}

void readBlock32()
{
	char dest[512];
	int i;

	memset(dest, 0, 512);
	ata->read_block(ata, 32, dest);

	printk("done reading block 32!\n");
	for (i = 0; i < 512; i += 8) {
		printk("%d] %lx\n", i, *((uint64_t *) (dest + i)));
	}
	kexit();
}

/* void configure_kmain_from_env(){ */
/*     if(getenv("KMAIN_NODEBUG")){ */
/* 	no_debug = 1; */
/*     } */

/*     char *print_level = getenv("KMAIN_PRINTLEVEL"); */
/*     if(print_level){ */
/* 	if(strcmp(print_level, "INFO") == 0){ */
/* 	    printk_conf = info; */
/* 	} */
/* 	else if(strcmp(print_level, "WARN") == 0){ */
/* 	    printk_conf = warn; */
/* 	} */
/* 	else if(strcmp(print_level, "ERR") == 0){ */
/* 	    printk_conf = err; */
/* 	} */
/* 	else if(strcmp(print_level, "SILENT") == 0){ */
/* 	    printk_conf = silent; */
/* 	} */
/*     } */
/* } */

void send_test_ethernet_frame(){
        hw_addr dest_mac = {.bytes={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
	hw_addr src_mac  = {.bytes={0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA}};
	uint16_t ethertype = ETHRTYPE_ARP;

	uint8_t data[28] = {
		0x00,
		0x01,
		0x08,
		0x00,
		0x06,
		0x04,
		0x00,
		0x01,
		0x52,
		0x54,
		0x00,
		0x60,
		0x7c,
		0x73,
		0xc0,
		0xa8,
		0x7a,
		0x01,
		0xff,
		0xff,
		0xff,
		0xff,
		0xff,
		0xff,
		0xc0,
		0xa8,
		0x7a,
		0x12
	};

	uint8_t *new_frame;
	int frame_len = create_ethernet_frame(dest_mac, src_mac, ethertype, data, 28, &new_frame);

	rtl8139_transmit_packet(new_frame, frame_len);
	kfree(new_frame);
}

void send_test_arp_request(){
	uint8_t *arp_request_frame;
	int arp_request_frame_length;
	
	/* uint32_t my_ipv4 = 172; */
	/* my_ipv4 <<= 8; */
	/* my_ipv4 += 16; */
	/* my_ipv4 <<= 8; */
	/* my_ipv4 += 210; */
	/* my_ipv4 <<= 8; */
	/* my_ipv4 += 203; */

	/* uint8_t my_mac[6] = {0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA}; */

	/* uint32_t target_ipv4 = 172; */
	/* target_ipv4 <<= 8; */
	/* target_ipv4 += 16; */
	/* target_ipv4 <<= 8; */
	/* target_ipv4 += 210; */
	/* target_ipv4 <<= 8; */
	/* target_ipv4 += 175; */

	uint32_t my_ipv4 = 192;
	my_ipv4 <<= 8;
	my_ipv4 += 168;
	my_ipv4 <<= 8;
	my_ipv4 += 122;
	my_ipv4 <<= 8;
	my_ipv4 += 16;

        /* hw_addr my_mac = {.bytes={0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA}}; */

	uint32_t target_ipv4 = 192;
	target_ipv4 <<= 8;
	target_ipv4 += 168;
	target_ipv4 <<= 8;
	target_ipv4 += 122;
	target_ipv4 <<= 8;
	target_ipv4 += 1;
	
        arp_request_frame_length = create_ipv4_arp_request(my_ipv4, target_ipv4, &arp_request_frame);

	rtl8139_transmit_packet(arp_request_frame, arp_request_frame_length);
}

void send_test_arp_reply(){
	uint8_t *arp_reply_frame;
	int arp_reply_frame_length;
	
	uint32_t my_ipv4 = 192;
	my_ipv4 <<= 8;
	my_ipv4 += 168;
	my_ipv4 <<= 8;
	my_ipv4 += 122;
	my_ipv4 <<= 8;
	my_ipv4 += 16;

        hw_addr my_mac = {.bytes={0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA}};
	
	uint32_t target_ipv4 = 192;
	target_ipv4 <<= 8;
	target_ipv4 += 168;
	target_ipv4 <<= 8;
	target_ipv4 += 122;
	target_ipv4 <<= 8;
	target_ipv4 += 1;

	hw_addr target_mac = {.bytes={0x52, 0x54, 0x00, 0x60, 0x7c, 0x73}};
	
        arp_reply_frame_length = create_ipv4_arp_reply(my_ipv4, my_mac, target_ipv4, target_mac, &arp_reply_frame);

	rtl8139_transmit_packet(arp_reply_frame, arp_reply_frame_length);
}


int kmain(void *multiboot_point, unsigned int multitest)
{
	TagStructureInfo *tagStructureInfo;
	GenericTagHeader *curTag;
	uint64_t ****new_page_table;
	int num_pci_devs;
	int enabled = 0;

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

	if (num_pci_devs) {
		init_pci_devices();
	}

	while(1){
		while (!enabled) ;
		send_test_arp_request();
		send_test_arp_reply();
		enabled = 0;

		/* while (!enabled) ; */
		/* send_test_arp_reply(); */
		/* enabled = 0; */
	}
	/* uint64_t initFAT32Params[2] = {2, (uint64_t)ata}; */
	/* PROC_create_kthread(initFAT32, initFAT32Params); */

	while (1) {
		PROC_run();
		asm("hlt");
	}

	return 0;
}
