#include <stdint-gcc.h>
#include "net/ip/icmp.h"
#include "net/ip/ipv4.h"
#include "net/ip/checksum.h"
#include "utils/printk.h"
#include "utils/byte_order.h"

int handle_received_icmp_packet(ipv4_header *ip_head){
	icmp_header_base *icmp_base = (icmp_header_base*)(((uint8_t*)ip_head) + sizeof(ipv4_header));

	icmp_type t = icmp_base->type;
	switch(t){
	case ICMP_TYPE_ECHO_REPLY:
		printk_debug("received ICMP ECHO REPLY\n");
		break;
	case ICMP_TYPE_ECHO_REQUEST:
		printk_debug("received ICMP ECHO REQUEST\n");
		break;
	default:
		printk_debug("received ICMP packet with unknown type (0x%x)\n", t);
		break;
	}

	return 0;
}

int create_icmp_echo_packet(ipv4_addr source, ipv4_addr dest, icmp_type echo_type, uint16_t identifier, uint16_t sequence_num, uint8_t *payload, uint32_t payload_len, uint8_t **packet_return){
	uint8_t packet[IPV4_DATA_LEN];
        icmp_echo_header *header = (icmp_echo_header*)packet;

	if(payload_len > IPV4_DATA_LEN){
		printk_warn("attempted to send udp packet with data requiring multiple, fragmented ipv4 packets. Currently not supported/n");
		*packet_return = (void*)0;
		return 0;
	}

	uint32_t total_packet_len = sizeof(icmp_echo_header) + payload_len;
	
	header->base.type = echo_type;
	header->base.code = 0; //code is 0 for both icmp request/reply
	header->base.checksum = 0;

	header->identifier = htons(identifier);
	header->sequence_num = htons(sequence_num);

	uint8_t *payload_start = packet + sizeof(icmp_echo_header);
	int i;
	for(i = 0; i < payload_len; i++){
		payload_start[i] = payload[i];
	}

	//calculate checksum
	unsigned short cksum = in_cksum((unsigned short*)packet, total_packet_len);
	header->base.checksum = cksum;

	return create_ipv4_packet(IPV4_PROTO_ICMP, source, dest, packet, total_packet_len, packet_return);
}
