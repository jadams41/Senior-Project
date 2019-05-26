#include <stdint-gcc.h>
#include "utils/printk.h"
#include "net/ip/udp.h"
#include "utils/byte_order.h"

int create_udp_packet(ipv4_addr source, udp_port source_port, ipv4_addr dest, udp_port dest_port, uint8_t *data, uint32_t data_len, uint8_t **packet_return){
	uint8_t packet[IPV4_DATA_LEN];
	udp_header *head = (udp_header*)packet;

	if(data_len > IPV4_DATA_LEN){
		printk_warn("attempted to send udp packet with data requiring multiple, fragmented ipv4 packets. Currently not supported/n");
		*packet_return = (void*)0;
		return 0;
	}

	uint32_t total_packet_len = sizeof(udp_header) + data_len;
	
	head->source_port = htons(source_port);
	head->dest_port = htons(dest_port);
	head->length = htons(total_packet_len);
	head->checksum = 0;

	/* copy data into packet */
	uint8_t *packet_data = packet + sizeof(udp_header);
	int i;
	for(i = 0; i < data_len; i++){
	        packet_data[i] = data[i];
	}
	
	return create_ipv4_packet(IPV4_PROTO_UDP, source, dest, packet, total_packet_len, packet_return);
}
