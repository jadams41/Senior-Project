#include <stdint-gcc.h>
#include "drivers/net/ethernet/realtek/8139too.h"
#include "net/ip/checksum.h"
#include "net/ip/ipv4.h"
#include "net/ip/tcp.h"
#include "utils/byte_order.h"
#include "utils/printk.h"

static int set_data_offset_and_flags(tcp_header *head, uint8_t data_offset, tcp_flags flags);
static int create_tcp_packet(ipv4_addr src, ipv4_addr dest, uint16_t src_port, uint16_t dest_port, uint32_t seq_num, uint32_t ack_num, uint8_t data_offset, tcp_flags flags, uint16_t win_size, uint16_t urgent_ptr, uint8_t *data, uint32_t data_len, uint8_t **packet_return);

int test_tcp_syn(){
	ipv4_addr src = str_to_ipv4("172.16.210.193"); //static ip of this machine
	ipv4_addr dest = str_to_ipv4("64.4.54.254");   //random, ripped from wireshark capture
	uint16_t src_port = 57746; //random, ripped from wireshark capture
	uint16_t dest_port = 443; //random, ripped from wireshark capture
	
	return establish_tcp_connection(src, dest, src_port, dest_port);
}

int establish_tcp_connection(ipv4_addr src, ipv4_addr dest, uint16_t src_port, uint16_t dest_port){
	uint8_t *tcp_frame;
	int tcp_frame_len;
	tcp_flags flags;

	uint32_t seq_num = 1;
	uint32_t ack_num = 0;
	uint16_t win_size = 29200; //random, taken from wireshark capture of tcp traffic

	flags.ns = 0;
	flags.cwr = 0;
	flags.ece = 0;
	flags.urg = 0;
	flags.ack = 0;
	flags.psh = 0;
	flags.rst = 0;
	flags.syn = 1;
	flags.fin = 0;

	//send SYN request to server
	tcp_frame_len = create_tcp_packet(src, dest, src_port, dest_port, seq_num, ack_num, (sizeof(tcp_header) /  4), flags, win_size, 0, NULL, 0, &tcp_frame);
	rtl8139_transmit_packet(tcp_frame, tcp_frame_len);

	return 0;
}


static int set_data_offset_and_flags(tcp_header *head, uint8_t data_offset, tcp_flags flags){
	uint16_t combined = 0;
	
	if((data_offset & 0b11110000) != 0){
		printk_err("bad data offset (field is only 4 bits)\n");
		return 1;
	}

	combined = data_offset; //put data offset in lowest 4 bits
	combined <<= 12; //move data offset into highest 4 bits (where this info is expected)

	/* reserved bits should already be correctly set to 0 */

	/* set flags in combined */
	//todo replace with more efficient logic (extra rigorous to get things working initially)
	if(flags.ns)
		combined |= 0b0000000100000000;
	if(flags.cwr)
		combined |= 0b0000000010000000;
	if(flags.ece)
		combined |= 0b0000000001000000;
	if(flags.urg)
		combined |= 0b0000000000100000;
	if(flags.ack)
		combined |= 0b0000000000010000;
	if(flags.psh)
		combined |= 0b0000000000001000;
	if(flags.rst)
		combined |= 0b0000000000000100;
	if(flags.syn)
		combined |= 0b0000000000000010;
	if(flags.fin)
		combined |= 0b0000000000000001;

	printk("combined = 0x%x\n", combined);
	
	head->data_offset_and_flags = htons(combined);
	return 0;
}

static int create_tcp_packet(ipv4_addr src, ipv4_addr dest, uint16_t src_port, uint16_t dest_port, uint32_t seq_num, uint32_t ack_num, uint8_t data_offset, tcp_flags flags, uint16_t win_size, uint16_t urgent_ptr, uint8_t *data, uint32_t data_len, uint8_t **packet_return){
	uint8_t packet[IPV4_DATA_LEN + sizeof(tcp_pseudo_header)];

	tcp_pseudo_header *pseudo = (tcp_pseudo_header*)packet;
	tcp_header *header = (tcp_header*)(packet + sizeof(tcp_pseudo_header));
	uint8_t *data_begin = packet + sizeof(tcp_pseudo_header) + sizeof(tcp_header);
	
	if(data_len > (IPV4_DATA_LEN - sizeof(tcp_header))){
		printk_warn("attempted to send tcp packet with too much data/n");
		*packet_return = (void*)0;
		return 0;
	}

	uint32_t total_packet_len = sizeof(tcp_header) + data_len;

	header->src_port = htons(src_port);
	header->dest_port = htons(dest_port);
	header->seq_num = htonl(seq_num);
	header->ack_num = htonl(ack_num);

	set_data_offset_and_flags(header, data_offset, flags);

	header->win_size = htons(win_size);
	header->urgent_ptr = htons(urgent_ptr);
	header->checksum = 0; //will set later
	header->urgent_ptr = htons(urgent_ptr);

	//copy data into packet (todo replace w/ memcpy)
	int i;
	for(i = 0; i < data_len; i++){
		data_begin[i] = data[i];
	}
	
	//prepare pseudo header for calculating checksum
	pseudo->source_ip = htonl(src);
	pseudo->dest_ip = htonl(dest);
	pseudo->reserved = 0;
	pseudo->protocol = IPV4_PROTO_TCP;
	pseudo->tcp_seg_len = htons(sizeof(tcp_pseudo_header) + sizeof(tcp_header) + data_len);

	//calculate checksum over pseudo_header, tcp_header, and data
	unsigned short cksum = in_cksum((unsigned short*)packet, sizeof(tcp_pseudo_header) + total_packet_len);
	header->checksum = htons(cksum);

	printk("data_offset and flags = 0x%x\n", header->data_offset_and_flags);
	
	return create_ipv4_packet(IPV4_PROTO_TCP, src, dest, (uint8_t*)header, total_packet_len, packet_return);
}
