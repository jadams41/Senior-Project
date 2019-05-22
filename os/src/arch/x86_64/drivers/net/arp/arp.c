#include <stdint-gcc.h>
#include "arp.h"
#include "drivers/net/ethernet/ethernet.h"
#include "utils/byte_order.h"

/**
  * @hrd hardware type (i.e ethernet)
  * @pro protocol type (i.e IPv4)
  * @hln hardware address length (i.e. ethernet's hln = 6 (mac address length))
  * @pln protocol address length (i.e. IPv4's pln = 4)
  * @op  opcode
  * @sha sender hardware address. Must be array of bytes w/ length equal to value specified in @hln
  * @spa sender protocol address. Must be array of bytes w/ length equal to value specified in @pln
  * @tha target hardware address. Must be array of bytes w/ length equal to value specified in @hln
  * @tpa target protocol address. Must be array of bytes w/ length equal to value specified in @pln

  * @packet_return if packet successfully created, will contain the address of the newly created packet upon return

  * @return length of created arp packet */
static int create_arp_packet(uint16_t hrd, uint16_t pro, uint8_t hln, uint8_t pln, uint16_t op, uint8_t *sha, uint8_t *spa, uint8_t *tha, uint8_t *tpa, uint8_t **packet_return){
	uint8_t arp_packet[ETH_DATA_LEN]; //max possible ethernet frame payload size
	uint64_t arp_packet_len = sizeof(arp_packet_base) + (2 * hln) + (2 * pln);
	int i;
	
	/* first create arp packet */
	arp_packet_base *base = (arp_packet_base*)arp_packet;
	base->hw_type = htons(hrd);
	base->proto = htons(pro);
	base->hw_addr_len = hln;
	base->proto_addr_len = pln;
	base->opcode = htons(op);

	//populate variable fields
	uint8_t *sender_hw_addr = (uint8_t*)(arp_packet + sizeof(arp_packet_base));
	uint8_t *sender_proto_addr = sender_hw_addr + hln;
	uint8_t *target_hw_addr = sender_proto_addr + pln;
	uint8_t *target_proto_addr = target_hw_addr + hln;
	
	for(i = 0; i < hln; i++){
		sender_hw_addr[i] = sha[i];
		target_hw_addr[i] = tha[i];
	}


	for(i = 0; i < pln; i++){
		sender_proto_addr[i] = spa[i];
		target_proto_addr[i] = tpa[i];
	}

	/* finally, create an ethernet frame with the new arp packet as data */
	return create_ethernet_frame(tha, sha, ETHRTYPE_ARP, arp_packet, arp_packet_len, packet_return);
}

int create_ipv4_arp_request(uint32_t my_ipv4, uint8_t *my_mac, uint32_t target_ipv4, uint8_t **packet_return){
	uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	uint32_t my_converted_ipv4     = htonl(my_ipv4);
	uint32_t target_converted_ipv4 = htonl(target_ipv4);
	
	return create_arp_packet(
		ARP_HW_ETHER,
		ARP_PROTO_IPV4,
		ARP_HW_ADDR_LEN_ETH,
		ARP_PROTO_ADDR_LEN_IPV4,
		ARP_OP_REQUEST,
	        my_mac,
	        (uint8_t*)(&my_converted_ipv4),
		dest_mac,
	        (uint8_t*)(&target_converted_ipv4),
		packet_return);
}

int create_ipv4_arp_reply(uint32_t my_ipv4, uint8_t *my_mac, uint32_t target_ipv4, uint8_t *target_mac, uint8_t **packet_return){
	uint32_t my_converted_ipv4     = htonl(my_ipv4);
	uint32_t target_converted_ipv4 = htonl(target_ipv4);

	return create_arp_packet(
		ARP_HW_ETHER,
		ARP_PROTO_IPV4,
		ARP_HW_ADDR_LEN_ETH,
		ARP_PROTO_ADDR_LEN_IPV4,
		ARP_OP_REPLY,
	        my_mac,
	        (uint8_t*)(&my_converted_ipv4),
		target_mac,
	        (uint8_t*)(&target_converted_ipv4),
		packet_return);
}
