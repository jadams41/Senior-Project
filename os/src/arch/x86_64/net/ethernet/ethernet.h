#ifndef ETHERNET
#define ETHERNET

#include <stdint-gcc.h>

#define ETH_ALEN      6    /* Octets in one ethernet addr */
#define ETH_HLEN      14   /* Total Octets in ethernet header */
#define ETH_ZLEN      60   /* Min Octets in frame sans Frame Check Sequence (FCS) */
#define ETH_DATA_LEN  1500 /* Max Octets in payload */
#define ETH_FRAME_LEN 1514 /* Max octets in frame sans Frame Check Sequence (FCS) */
#define ETH_FCS_LEN   4    /* Octets in the Frame Check Sequence (FCS) */

typedef struct {
	uint8_t bytes[ETH_ALEN];
} hw_addr;

enum Ethertypes {
	ETHRTYPE_IPV4 = 0x0800,
	ETHRTYPE_ARP  = 0x0806,
	ETHRTYPE_IPV6 = 0x86DD,
};

#define ETHRTYPE_IPV4_NAME    "IPV4"
#define ETHRTYPE_ARP_NAME     "ARP"
#define ETHRTYPE_IPV6_NAME    "IPV6"
#define ETHRTYPE_UNKNOWN_NAME "Unknown"

typedef struct {
	uint8_t dest_mac[ETH_ALEN];
	uint8_t src_mac[ETH_ALEN];
	uint16_t ethertype;
}__attribute__((packed)) eth_frame_header;

/* hw address functions */
hw_addr broadcast_mac_addr();
hw_addr create_hw_addr(uint8_t *hw_addr_ptr);
int hw_addr_compare(hw_addr addr1, hw_addr addr2);
char *hw_addr_to_str(hw_addr addr);

/* ethertype functions */
char *ethertype_to_str(uint32_t ethertype);

/* functions for creating/handling ethernet traffic */
int create_ethernet_frame(hw_addr dest, hw_addr src, uint16_t ethertype, uint8_t *data, uint64_t data_len, uint8_t **frame_return);
void print_eth_frame(uint8_t *eth_frame, unsigned int frame_size);
void handle_eth_frame(uint8_t *eth_frame, unsigned int frame_size);

#endif
