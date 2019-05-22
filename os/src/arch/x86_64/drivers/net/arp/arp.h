#ifndef ARP
#define ARP

typedef struct {
	uint16_t hw_type;       /* Hardware Type - specifies the type of 
				   hardware used for the local network 
				   transmitting the ARP message; thus, it also
				   identifies the type of addressing used. */
	
	uint16_t proto;	        /* Protocol Type - Complement of the Hardware 
				   Type field, specifying the type of layer 3 
				   addresses used in the message. */
	
	uint8_t hw_addr_len;	/* Hardware Address Length - specifies how long
				   the hardware addresses are in this message.
				   For Ethernet or other networks using IEEE 
				   802 MAC addresses, this value is 6 */

	uint8_t proto_addr_len; /* Protocol Address Length - Specifies how long
				   protocol (layer 3) addresses are in this
				   message */

	uint16_t opcode;        /* Opcode - Specifies the nature of the ARP
				   message being sent. */
} __attribute__ ((packed)) arp_packet_base;

/* typedef struct { */
/* 	arp_packet_base base; */

/* 	uint_8 sender_hw_addr[6]; */
/* 	uint_8 sender_proto_addr[4]; */
	
	
/* } __attribute__ ((packed)) arp_packet_ipv4; */

/**** acessible functions ****/
int create_ipv4_arp_request(uint32_t my_ipv4, uint8_t *my_mac, uint32_t target_ipv4, uint8_t **packet_return);
int create_ipv4_arp_reply(uint32_t my_ipv4, uint8_t *my_mac, uint32_t target_ipv4, uint8_t *target_mac, uint8_t **packet_return);

/**** Allowed values for different fields in the packet ****/
/* NOTE: taken from http://www.tcpipguide.com/free/t_ARPMessageFormat.htm */
enum arp_hw_types {
	ARP_HW_ETHER         = 1,
	ARP_HW_IEEE_802_NET  = 6,
	ARP_HW_ARCNET        = 7,
	ARP_HW_FRAME_RELAY   = 15,
	ARP_HW_ATM1          = 16,
	ARP_HW_HDLC          = 17,
	ARP_HW_FIBRE_CHANNEL = 18,
	ARP_HW_ATM2          = 19,
	ARP_HW_SERIAL_LINE   = 20
};

/* corresponds to ethernet ethertypes for different protocol addresses.
 * todo: fill in more values as necessary (can't find list of all allowed values) */
enum arp_protocol_types {
	ARP_PROTO_IPV4 = 0x0800,
	ARP_PROTO_IPV6 = 0x86DD
};

enum arp_hw_addr_lengths {
	ARP_HW_ADDR_LEN_ETH = 6
};

enum arp_proto_addr_lengths {
	ARP_PROTO_ADDR_LEN_IPV4 = 4,
	ARP_PROTO_ADDR_LEN_IPV6 = 16
};

enum arp_opcodes {
	ARP_OP_REQUEST       = 1,
	ARP_OP_REPLY         = 2,
	ARP_OP_RARP_REQUEST  = 3,
	ARP_OP_RARP_REPLY    = 4,
	ARP_OP_DRARP_REQUEST = 5,
	ARP_OP_DRARP_REPLY   = 6,
	ARP_OP_DRARP_ERROR   = 7,
	ARP_OP_INARP_REQUEST = 8,
	ARP_OP_INARP_REPLY   = 9
};

#endif
