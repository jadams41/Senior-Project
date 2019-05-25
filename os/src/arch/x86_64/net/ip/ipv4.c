#include <stdint-gcc.h>
#include "net/ethernet/ethernet.h"
#include "net/ethernet/realtek/8139too.h"
#include "net/ip/ipv4.h"
#include "net/ip/checksum.h"
#include "utils/byte_order.h"
#include "utils/printk.h"


extern rt8139_private *global_rtl_priv;

static void ipv4_to_str(uint32_t ipv4, char *out){
	uint8_t ipv4_addr1 = ((uint8_t*)&ipv4)[3];
	uint8_t ipv4_addr2 = ((uint8_t*)&ipv4)[2];
	uint8_t ipv4_addr3 = ((uint8_t*)&ipv4)[1];
	uint8_t ipv4_addr4 = ((uint8_t*)&ipv4)[0];
	
	sprintk(out, "%u.%u.%u.%u", ipv4_addr1, ipv4_addr2, ipv4_addr3, ipv4_addr4);

}


static int set_version(ipv4_header *header, uint8_t version){
	if((version & 0xF) != version){
		printk_err("Bad IPv4 Version 0x%x (must be 4 bits)\n", version);
		return 1;
	}

	uint8_t packed = header->version__ihl;
	packed &= (~IPV4_PACKET_VER_MASK & ((version & 0x0F) << 4));
	header->version__ihl = packed;

	return 0;
}

static uint8_t get_version(ipv4_header *header){
	uint8_t version_and_ihl = header->version__ihl;
	uint8_t version = (version_and_ihl & IPV4_PACKET_VER_MASK) >> 4;
	return version;
}

static int set_ihl(ipv4_header *header, uint8_t ihl){
	if((ihl & 0xF) != ihl){
		printk_err("Bad IPv4 IHL 0x%x (must be 4 bits)\n", ihl);
		return 1;
	}
	
	uint8_t packed = header->version__ihl;
	packed &= (~IPV4_PACKET_IHL_MASK & (ihl & 0x0F));
	header->version__ihl = packed;
	return 0;
}

static uint8_t get_ihl(ipv4_header *header){
	uint8_t version_and_ihl = header->version__ihl;
	uint8_t ihl = (version_and_ihl & IPV4_PACKET_IHL_MASK);
	return ihl;
}

static int set_dscp(ipv4_header *header, uint8_t dscp){
	if((dscp & 0b00111111) != dscp){
		printk_err("Bad IPv4 DSCP 0x%x (must be 6 bits)\n", dscp);
		return 1;
	}
	
	uint8_t packed = header->dscp__ecn;
	packed &= (~IPV4_PACKET_DSCP_MASK & (dscp << 2));
	header->dscp__ecn = packed;
	return 0;
}

static uint8_t get_dscp(ipv4_header *header){
	uint8_t dscp_and_ecn = header->dscp__ecn;
	uint8_t dscp = (dscp_and_ecn & IPV4_PACKET_DSCP_MASK) >> 2;
	return dscp;
}

static int set_ecn(ipv4_header *header, uint8_t ecn){
	if((ecn & 0b00000011) != ecn){
		printk_err("Bad IPv4 ECN 0x%x (must be 2 bits)\n", ecn);
		return 1;
	}
	
	uint8_t packed = header->dscp__ecn;
	packed &= (~IPV4_PACKET_ECN_MASK & ecn);
	header->dscp__ecn = packed;
	return 0;
}

static uint8_t get_ecn(ipv4_header *header){
	uint8_t dscp_and_ecn = header->dscp__ecn;
	uint8_t ecn = (dscp_and_ecn & IPV4_PACKET_ECN_MASK);
	return ecn;
}

static int set_flags(ipv4_header *header, uint8_t dont_frag, uint8_t more_frag){
	int err = 0;

	if((dont_frag & 0b00000001) != dont_frag){
		printk_err("Bad dont_frag flag 0x%x (must be 1 bits)\n", dont_frag);
		err = 1;
	}
	if((more_frag & 0b00000001) != more_frag){
		printk_err("Bad more_frag flag 0x%x (must be 1 bits)\n", more_frag);
		err = 1;
	}
	if(err){
		return 1;
	}
	
	uint8_t packed = header->flags__frag_off;
	packed &= ~IPV4_FLAGS_RESERVED_MASK; //set reserved flag = 0
	packed &= (~IPV4_FLAGS_DONTFRAG_MASK & (dont_frag << 14));
	packed &= (~IPV4_FLAGS_MOREFRAG_MASK & (more_frag << 13));

	header->flags__frag_off = packed;
	return 0;
}

static uint8_t get_flag_dontfrag(ipv4_header *header){
	uint16_t flags_and_fragoff = ntohs(header->flags__frag_off);
	return (flags_and_fragoff & IPV4_FLAGS_DONTFRAG_MASK) != 0;
}

static uint8_t get_flag_morefrag(ipv4_header *header){
	uint16_t flags_and_fragoff = ntohs(header->flags__frag_off);
	return (flags_and_fragoff & IPV4_FLAGS_MOREFRAG_MASK) != 0;
}

static int set_frag_off(ipv4_header *header, uint16_t frag_off){
	if((frag_off & IPV4_FRAGMENTOFFSET_MASK) != frag_off){
		printk_err("Bad fragmentation offset 0x%x (must be 13 bits)\n", frag_off);
		return 1;
	}
	
	
	uint16_t packed = header->flags__frag_off;
	packed &= (~IPV4_FRAGMENTOFFSET_MASK & frag_off);

	header->flags__frag_off = packed;
	return 0;
}

static uint16_t get_frag_off(ipv4_header *header){
	uint16_t flags_and_fragoff = ntohs(header->flags__frag_off);
	return (flags_and_fragoff & IPV4_FRAGMENTOFFSET_MASK);
}

void handle_received_ip_packet(uint8_t *frame, unsigned int frame_len){
	//eth_frame_header *frame_head = (eth_frame_header*)frame;
	ipv4_header *ip_head = (ipv4_header*)(frame + sizeof(eth_frame_header));
	char source_ip_str[16];
	char dest_ip_str[16];

	uint8_t version, ihl, dscp, ecn;
	uint16_t fragmentation_offset;
	
	ihl = get_ihl(ip_head);
        version = get_version(ip_head);
	dscp = get_dscp(ip_head);
	ecn = get_ecn(ip_head);
	fragmentation_offset = get_frag_off(ip_head);
	
	switch(version){
	case IPV4_VERSION:
		printk("--------------- Received IPv4 Packet ---------------\n");
		printk("\tHeader Length = %u\n", ihl * 4);
		printk("\tDSCP = 0x%x\n", dscp);
		printk("\tECN = 0b%d%d\n", (ecn & 0b10) >> 1, ecn & 0b01);
		printk("\tTotal packet length = %d bytes\n", ntohs(ip_head->total_len));
		printk("\tId = %u\n", ntohs(ip_head->id));
		printk("\tFlags: Don't Fragment=%d, More Fragments=%d\n", get_flag_dontfrag(ip_head), get_flag_morefrag(ip_head));
		printk("\tFragmentation Offset = %u\n", fragmentation_offset);
		printk("\tTime to Live = %u\n", ip_head->ttl);

		switch(ip_head->protocol){
		case IPV4_PROTO_ICMP:
			printk("\tProtocol: ICMP\n");
			break;
		case IPV4_PROTO_TCP:
			printk("\tProtocol: TCP\n");
			break;
		case IPV4_PROTO_UDP:
			printk("\tProtocol: UDP\n");
			break;
		default:
			printk("\tProtocol: Unrecognized (0x%x)\n", ip_head->protocol);
			break;
		}

		printk("\tHeader Checksum: 0x%x\n", ntohs(ip_head->header_check));

	        ipv4_to_str(ntohl(ip_head->source), source_ip_str);
		ipv4_to_str(ntohl(ip_head->dest), dest_ip_str);
		printk("Source IPv4 address: %s\n", source_ip_str);
		printk("Destination IPv4 address: %s\n", dest_ip_str);
		break;
	default:
		printk_warn("received ip packet with version=%u don't know how to handle!\n", version);
		break;
	}
}

/**
   @protocol ipv4 protocol for the included data
   @source   source ipv4 address (expected to be in host order)
   @dest     destination ipv4 address (expected to be in host order)
   @data     data to include in the packet
   @data_len length of data to include in packet
   @packet_return upon successful packet creation, will contain pointer to created packet
*/
int create_ipv4_packet(uint8_t protocol, uint32_t source, uint32_t dest, uint8_t *data, uint64_t data_len, uint8_t **packet_return){
	uint8_t packet[ETH_DATA_LEN];
	ipv4_header *head = (ipv4_header*)packet;
	
	if(data_len > (ETH_DATA_LEN - (IPV4_MIN_IHL * 4))){
		printk_warn("attempted to send ipv4 packet with data requiring multiple, fragmented packets. Currently not supported/n");
		*packet_return = (void *)0;
		return 0;
	}

	uint64_t packet_len = data_len + (IPV4_MIN_IHL * 4);

	uint8_t version = IPV4_VERSION;
	uint8_t ihl = IPV4_MIN_IHL;
	
        if(set_version(head, version) ||
	   set_ihl(head, ihl) ||
	   set_dscp(head, IPV4_DSCP_DF) ||
	   set_ecn(head, IPV4_ECN_NONECT) ||
	   set_flags(head, 0, 0) ||
	   set_frag_off(head, htonl(IPV4_FRAGOFF_FIRST))){
		return 0;
	}
	head->total_len = htons((ihl * 4) + data_len);
	head->id = 0; //I think this should work?

	head->ttl = IPV4_TTL_RECOMMENDED;
	head->protocol = protocol;

	//this is only being set to 0 temporarily (in order to checksum the other fields in the packet)
	head->header_check = 0;
	
	head->source = htonl(source);
	head->dest = htonl(dest);

	//calculate checksum
	unsigned short checksum = in_cksum((unsigned short*)head, ihl * 4);
	head->header_check = htons(checksum);

	//copy data into packet
	int i;
	uint8_t *packet_data = packet + sizeof(ipv4_header);
	for(i = 0; i < data_len; i++){
	        packet_data[i] = data[i];
	}

	hw_addr hw_dest = broadcast_mac_addr(); //todo replace this with router's mac when I figure out how to get that information
	hw_addr hw_src = global_rtl_priv->mac_addr;
	
	/* finally, create an ethernet frame with the new ipv4 packet as data */
	return create_ethernet_frame(hw_dest, hw_src, ETHRTYPE_IPV4, packet, packet_len, packet_return);
}