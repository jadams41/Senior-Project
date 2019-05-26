#ifndef ICMP
#define ICMP

#include <stdint-gcc.h>
#include "net/ip/ipv4.h"

typedef uint8_t icmp_type;
typedef uint8_t icmp_code;

typedef struct {
	icmp_type type;
	icmp_code code;
	uint16_t checksum;
}__attribute__((packed)) icmp_header_base;

typedef struct {
	icmp_header_base base;
	uint16_t identifier;
	uint16_t sequence_num;
}__attribute__((packed)) icmp_echo_header;

int handle_received_icmp_packet(ipv4_header *ip_head);
int create_icmp_echo_packet(ipv4_addr source, ipv4_addr dest, icmp_type echo_type, uint16_t identifier, uint16_t sequence_num, uint8_t *payload, uint32_t payload_len, uint8_t **packet_return);

#define ICMP_TYPE_ECHO_REPLY 0
#define ICMP_TYPE_ECHO_REQUEST 8

#endif
