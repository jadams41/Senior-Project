#include <stdint-gcc.h>
#include "net/ip/dhcp.h"
#include "drivers/net/ethernet/realtek/8139too.h"

extern rt8139_private *global_rtl_priv;

/* DHCP message for requesting an ip address 
 * NOTE: Sloppy! Can't seem to find rigid packet formats or any well-written 
         dhcp code anywhere online?
 * NOTE: logic taken from: https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
 */
/* static int create_dhcp_discover(uint8_t **packet_return){ */
/* 	uint8_t packet[IPV4_DATA_LEN]; */
/* 	uint32_t *long_field; */
/* 	dhcp_packet_base *base = (dhcp_packet_base*)packet; */
/* 	int i; */
/* 	hw_addr hw_src = global_rtl_priv->mac_addr; */
	
/* 	base->opcode = 0x01; */
/* 	base->h_type = 0x01; */
/* 	base->h_len = 0x06; */
/* 	base->hops = 0x00; */
/* 	base->xid = htonl(0x3903F326); //this might be wrong */
/* 	base->secs = 0; */
/* 	base->flags = 0; */
/* 	base->ciaddr = 0; */
/* 	base->yiaddr = 0; */
/* 	base->siaddr = 0; */
/* 	base->giaddr = 0; */

/* 	//copy over my mac address */
/* 	for(i = 0; i < 6; i++){ */
/* 		base->ch_addr[i] = hw_src.bytes[i]; */
/* 	} */
/* 	//zero out the rest of the ch_addr */
/* 	for(i = 6; i < 16; i++){ */
/* 		base->ch_addr[i] = 0; */
/* 	} */

/*         //zero out s_name and file fields */
/* 	for(i = 0; i < 64; i++){ */
/* 		base->s_name[i] = 0; */
/* 	} */
/* 	for(i = 0; i < 128; i++){ */
/* 		base->file[i] = 0; */
/* 	} */

/* 	//set Magic Cookie (?) */
/* 	long_field = (uint32_t*)(packet + sizeof(dhcp_packet_base)); */
/* 	*long_field = htonl(0x63825363); */

/* 	//set options for DHCPDISCOVER message */
/* 	long_field += 1; */
	
	
/* 	return create_ipv4_packet(IPV4_PROTO_UDP, source, dest, packet, total_packet_len, packet_return); */
/* } */
