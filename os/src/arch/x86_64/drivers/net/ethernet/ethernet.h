#ifndef ETHERNET
#define ETHERNET

#define ETH_ALEN      6    /* Octets in one ethernet addr */
#define ETH_HLEN      14   /* Total Octets in ethernet header */
#define ETH_ZLEN      60   /* Min Octets in frame sans Frame Check Sequence (FCS) */
#define ETH_DATA_LEN  1500 /* Max Octets in payload */
#define ETH_FRAME_LEN 1514 /* Max octets in frame sans Frame Check Sequence (FCS) */
#define ETH_FCS_LEN   4    /* Octets in the Frame Check Sequence (FCS) */

enum Ethertypes {
	ETHRTYPE_IPV4 = 0x0800,
	ETHRTYPE_ARP  = 0x0806,
	ETHRTYPE_IPV6 = 0x86DD,
};

typedef struct {
	uint8_t dest_mac[ETH_ALEN];
	uint8_t src_mac[ETH_ALEN];
	uint16_t ethertype;
}__attribute__((packed)) eth_frame_header;

int create_ethernet_frame(uint8_t *dest, uint8_t *src, uint16_t ethertype, uint8_t *data, uint64_t data_len, uint8_t **frame_return);


#endif
