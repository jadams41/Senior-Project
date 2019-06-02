#include <stdint-gcc.h>
#include "arp.h"
#include "drivers/net/ethernet/realtek/8139too.h"
#include "net/ethernet/ethernet.h"
#include "drivers/memory/memoryManager.h"
#include "utils/byte_order.h"
#include "utils/printk.h"

static arp_table_entry *arp_table = (arp_table_entry*)0;

extern rt8139_private *global_rtl_priv;

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
	return create_ethernet_frame(create_hw_addr(tha), global_rtl_priv->mac_addr, ETHRTYPE_ARP, arp_packet, arp_packet_len, packet_return);
}

int create_ipv4_arp_request(uint32_t my_ipv4, uint32_t target_ipv4, uint8_t **packet_return){
	hw_addr sender_mac = {.bytes={0,0,0,0,0,0}}; //left empty in request
	hw_addr dest_mac = broadcast_mac_addr();

	uint32_t my_converted_ipv4     = htonl(my_ipv4);
	uint32_t target_converted_ipv4 = htonl(target_ipv4);
	
	return create_arp_packet(
		ARP_HW_ETHER,
		ARP_PROTO_IPV4,
		ARP_HW_ADDR_LEN_ETH,
		ARP_PROTO_ADDR_LEN_IPV4,
		ARP_OP_REQUEST,
	        sender_mac.bytes,
	        (uint8_t*)(&my_converted_ipv4),
		dest_mac.bytes,
	        (uint8_t*)(&target_converted_ipv4),
		packet_return);
}

int create_ipv4_arp_reply(uint32_t my_ipv4, hw_addr my_mac, uint32_t target_ipv4, hw_addr target_mac, uint8_t **packet_return){
	uint32_t my_converted_ipv4     = htonl(my_ipv4);
	uint32_t target_converted_ipv4 = htonl(target_ipv4);

	return create_arp_packet(
		ARP_HW_ETHER,
		ARP_PROTO_IPV4,
		ARP_HW_ADDR_LEN_ETH,
		ARP_PROTO_ADDR_LEN_IPV4,
		ARP_OP_REPLY,
	        my_mac.bytes,
	        (uint8_t*)(&my_converted_ipv4),
		target_mac.bytes,
	        (uint8_t*)(&target_converted_ipv4),
		packet_return);
}

void print_arp_table(){
	/* arp_table_entry *cur = arp_table; */
	/* printk("Current arp table:\n"); */
	/* printk("%25s%8s%20s%6s%16s%6s\n", "Address", "HWtype", "HWaddress", "Flags", "Mask", "Iface"); */

	/* while(cur){ */
	/* 	uint8_t ipv4_addr1 = (cur->ip_addr >> 24) & 0xFF000000; */
	/* 	uint8_t ipv4_addr2 = (cur->ip_addr >> 16) & 0x00FF0000; */
	/* 	uint8_t ipv4_addr3 = (cur->ip_addr >> 8) & 0x0000FF00; */
	/* 	uint8_t ipv4_addr4 = cur->ip_addr & 0x000000FF; */

	/* 	char ip_addr_str[16]; */
	/*         sprintk(ip_addr_str, "%u.%u.%u.%u", ipv4_addr1, ipv4_addr2, ipv4_addr3, ipv4_addr4); */

	/* 	char *mac_addr_str = hw_addr_to_str(cur->hw_addr); */
	/* 	//printk("%25s%8s%20s%6s%16s%6s\n", ip_addr_str, ((cur->hw_type == ARP_HW_ETHER) ? "ether" : " "), mac_addr_str, " ", " ", " ",  " "); */

	/* 	kfree(mac_addr_str); */
	/* 	cur = cur->next; */
	/* } */
}

void print_ipv4(uint32_t ipv4){
	uint8_t ipv4_addr1 = ((uint8_t*)&ipv4)[3];
	uint8_t ipv4_addr2 = ((uint8_t*)&ipv4)[2];
	uint8_t ipv4_addr3 = ((uint8_t*)&ipv4)[1];
	uint8_t ipv4_addr4 = ((uint8_t*)&ipv4)[0];
	
	printk("%u.%u.%u.%u", ipv4_addr1, ipv4_addr2, ipv4_addr3, ipv4_addr4);

}

int add_arp_table_entry(uint32_t ip_addr, uint16_t hw_type, hw_addr hw_addr){
	/* check if ip address is already in the arp table before adding a new entry */
	arp_table_entry *cur = arp_table;
	while(cur){
		if(cur->ip_addr == ip_addr){
			//if hardware addr is different, update
			if(hw_addr_compare(hw_addr, cur->hw_addr)){
				cur->hw_addr = hw_addr;

				uint32_t ipv4 = cur->ip_addr;
				
				uint8_t ipv4_addr1 = ((uint8_t*)&ipv4)[3];
				uint8_t ipv4_addr2 = ((uint8_t*)&ipv4)[2];
				uint8_t ipv4_addr3 = ((uint8_t*)&ipv4)[1];
				uint8_t ipv4_addr4 = ((uint8_t*)&ipv4)[0];
				
				char ip_addr_str[16];
				sprintk(ip_addr_str, "%u.%u.%u.%u", ipv4_addr1, ipv4_addr2, ipv4_addr3, ipv4_addr4);
				
				//printk_info("updated arp table entry for %s\n", ip_addr_str);
				print_arp_table();
				return 2;
			}

			return 0;
		}
		cur = cur->next;
	}
	
	arp_table_entry *new_entry = (arp_table_entry*)kmalloc(sizeof(arp_table_entry));
	new_entry->ip_addr = ip_addr;
	new_entry->hw_type = hw_type;
	new_entry->hw_addr = hw_addr;

	if(!arp_table){
		new_entry->next = NULL;
		arp_table = new_entry;
	}
	else {
		new_entry->next = arp_table;
		arp_table = new_entry;
	}

	uint8_t ipv4_addr1 = ((uint8_t*)&ip_addr)[3];
	uint8_t ipv4_addr2 = ((uint8_t*)&ip_addr)[2];
	uint8_t ipv4_addr3 = ((uint8_t*)&ip_addr)[1];
	uint8_t ipv4_addr4 = ((uint8_t*)&ip_addr)[0];
	
	char ip_addr_str[16];
	sprintk(ip_addr_str, "%u.%u.%u.%u", ipv4_addr1, ipv4_addr2, ipv4_addr3, ipv4_addr4);
	
	//printk_info("added arp table entry for %s\n", ip_addr_str);
	print_arp_table();
	return 1;
}

void handle_received_arp_packet(uint8_t *frame, unsigned int frame_len){
	asm("CLI");
        eth_frame_header *frame_head = (eth_frame_header*)frame;
	arp_packet_base  *arp_packet = (arp_packet_base*)(frame + sizeof(eth_frame_header));

	//hw_addr dest = create_hw_addr(frame_head->dest_mac);
	hw_addr src = create_hw_addr(frame_head->src_mac);

	/* char *dest_addr_str = hw_addr_to_str(dest); */
	char *src_addr_str = hw_addr_to_str(src);

	uint16_t hw_type = ntohs(arp_packet->hw_type);
	uint8_t hw_addr_len = arp_packet->hw_addr_len;
	uint8_t proto_addr_len = arp_packet->proto_addr_len;
	uint16_t opcode = ntohs(arp_packet->opcode);
	
	switch(opcode){
	case ARP_OP_REQUEST:
	        printk_info("Received ARP REQUEST from %s\n", src_addr_str);
		/* printk_info("\tsent to %s\n", dest_addr_str); */
		/* printk_info("\thw_addr_len = %u\n", hw_addr_len); */
		/* printk_info("\tproto_addr_len = %u\n", proto_addr_len); */
		/* printk_info("\tmy ipv4 = %u\n", global_rtl_priv->ipv4_addr); */
		
		if(proto_addr_len == ARP_PROTO_ADDR_LEN_IPV4){
			uint8_t *sender_hw_addr = (uint8_t*)(arp_packet + 1);
			hw_addr sender_mac = create_hw_addr(sender_hw_addr);
			
			uint32_t sender_ipv4 = *((uint32_t*)(sender_hw_addr + hw_addr_len));
			uint32_t target_ipv4 = *((uint32_t*)(sender_hw_addr + (hw_addr_len * 2) + proto_addr_len));

			sender_ipv4 = ntohl(sender_ipv4);
			target_ipv4 = ntohl(target_ipv4);

			//printk_info("\tsent from ");
			//print_ipv4(sender_ipv4);
			//printk("| lookng for ");
			//print_ipv4(target_ipv4);
			//printk("\n");
			
			//add the information from the request to the arp table
			add_arp_table_entry(sender_ipv4, hw_type, sender_mac);

			//reply if sender is arping us
			if(target_ipv4 == global_rtl_priv->ipv4_addr){
			        uint8_t *reply_packet;
				uint8_t packet_len = create_ipv4_arp_reply(global_rtl_priv->ipv4_addr, global_rtl_priv->mac_addr, sender_ipv4, sender_mac, &reply_packet);

				//printk_info("arp request was for us, sending arp reply with our ip address\n");
				rtl8139_transmit_packet(reply_packet, packet_len);
			}
		}
		else {
			//printk_warn("received arp request with proto address length = %hu. Don't know how to handle this yet\n", proto_addr_len);
			return;
		}
		break;
	case ARP_OP_REPLY:
	        //printk_info("Received ARP REPLY from %s\n", src_addr_str);
		//printk_info("\tsent to %s\n", dest_addr_str);

		if(proto_addr_len == ARP_PROTO_ADDR_LEN_IPV4){
			hw_addr sender_mac = create_hw_addr((uint8_t*)(arp_packet + 1));
			uint32_t sender_ipv4 = ntohl(*((uint32_t*)((uint8_t*)(arp_packet + 1)) + hw_addr_len));
			//uint32_t target_ipv4 = ntohl(*((uint32_t*)((uint8_t*)(arp_packet + 1)) + (hw_addr_len * 2) + proto_addr_len));
			
			//add the information from the request to the arp table
			add_arp_table_entry(sender_ipv4, hw_type, sender_mac);

		}
		else {
			//printk_warn("received arp request with proto address length = %hu. Don't know how to handle this yet\n", proto_addr_len);
			return;
		}
		break;
	}

	asm("STI");
}
