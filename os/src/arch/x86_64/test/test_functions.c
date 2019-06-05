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

void send_test_ipv4(){
	uint8_t *ipv4_frame;
	int ipv4_frame_length;

	uint32_t my_ipv4 = 192;
	my_ipv4 <<= 8;
	my_ipv4 += 168;
	my_ipv4 <<= 8;
	my_ipv4 += 122;
	my_ipv4 <<= 8;
	my_ipv4 += 16;
	
	uint32_t target_ipv4 = 192;
	target_ipv4 <<= 8;
	target_ipv4 += 192;
	target_ipv4 <<= 8;
	target_ipv4 += 192;
	target_ipv4 <<= 8;
	target_ipv4 += 192;

	ipv4_frame_length = create_ipv4_packet(IPV4_PROTO_UDP, my_ipv4, target_ipv4, NULL, 0, &ipv4_frame);
	rtl8139_transmit_packet(ipv4_frame, ipv4_frame_length);
}

void send_test_udp(){
	uint8_t *udp_frame;
	int udp_frame_len;

	uint8_t data[10] = {0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

	ipv4_addr source = str_to_ipv4(STATIC_IP);

	ipv4_addr dest = str_to_ipv4("172.16.210.183");
	
	udp_frame_len = create_udp_packet(source, 55, dest, 106, data, 10, &udp_frame);
	rtl8139_transmit_packet(udp_frame, udp_frame_len);
}

void send_ping(){
	uint8_t *icmp_frame;
	int icmp_frame_len;

	char src_str[20];
	char dest_str[20];
	
	//bridge's ip address: 169.254.6.164
	ipv4_addr source = str_to_ipv4(STATIC_IP);
	
	ipv4_addr dest = str_to_ipv4("8.8.8.8");

	ipv4_to_str(source, src_str);
	ipv4_to_str(dest, dest_str);


	printk("about to ping from `%s` to `%s`\n", src_str, dest_str);
	
	icmp_frame_len = create_icmp_echo_packet(source, dest, ICMP_TYPE_ECHO_REQUEST, 0xBEEF, 1, NULL, 0, &icmp_frame);
	rtl8139_transmit_packet(icmp_frame, icmp_frame_len);
}
