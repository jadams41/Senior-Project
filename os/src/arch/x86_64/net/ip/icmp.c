#include <stdint-gcc.h>
#include "drivers/net/ethernet/realtek/8139too.h"
#include "net/ip/icmp.h"
#include "net/ip/ipv4.h"
#include "net/ip/checksum.h"
#include "utils/printk.h"
#include "utils/byte_order.h"

extern rt8139_private *global_rtl_priv;

static int respond_to_icmp_echo_request(icmp_echo_header *received_echo_req, ipv4_addr dest, uint32_t payload_len){
	/* information for the packet we are about to create */
	uint8_t *icmp_frame;
	int icmp_frame_len;

	/* information used to configure the packet header */
	uint16_t identifier = ntohs(received_echo_req->identifier);
	uint16_t sequence_num = ntohs(received_echo_req->sequence_num);
	uint8_t *data = (uint8_t*)((received_echo_req + 1));
	
	/* create icmp echo response (same id, same sequence number) */
	icmp_frame_len = create_icmp_echo_packet(global_rtl_priv->ipv4_addr, dest, ICMP_TYPE_ECHO_REPLY, identifier, sequence_num, data, payload_len, &icmp_frame);

	printk_debug("sending icmp response\n");
	
	/* send created icmp echo response back to sender */
	rtl8139_transmit_packet(icmp_frame, icmp_frame_len);

	return icmp_frame_len;
}

int handle_received_icmp_packet(ipv4_header *ip_head){
	ipv4_addr source = ntohl(ip_head->source);
	ipv4_addr dest = ntohl(ip_head->dest);
	uint16_t ipv4_total_len = ntohs(ip_head->total_len);
	uint16_t icmp_packet_size = ipv4_total_len - sizeof(ipv4_header);
	uint16_t icmp_payload_size;
	
	char source_ip_str[20];
	char dest_ip_str[20];

	ipv4_to_str(source, source_ip_str);
	ipv4_to_str(dest, dest_ip_str);
	
	icmp_header_base *icmp_base = (icmp_header_base*)(((uint8_t*)ip_head) + sizeof(ipv4_header));
	icmp_type t = icmp_base->type;
	icmp_echo_header *echo_h;
	
	switch(t){
	case ICMP_TYPE_ECHO_REPLY:
		printk_debug("received ICMP ECHO REPLY\n\tfrom %s\n\tto %s\n", source_ip_str, dest_ip_str);
	        echo_h = (icmp_echo_header*)(((uint8_t*)ip_head) + sizeof(ipv4_header));
		break;
	case ICMP_TYPE_ECHO_REQUEST:
		echo_h = (icmp_echo_header*)(((uint8_t*)ip_head) + sizeof(ipv4_header));
		icmp_payload_size = icmp_packet_size - sizeof(icmp_echo_header);
		printk_debug("received ICMP ECHO REQUEST\n\tfrom %s\n\tto %s\n", source_ip_str, dest_ip_str);

                // check if request is for us
		if(dest == global_rtl_priv->ipv4_addr){
			printk_info("request was for us, RESPONDING!\n");
			respond_to_icmp_echo_request(echo_h, source, icmp_payload_size);
		}

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
