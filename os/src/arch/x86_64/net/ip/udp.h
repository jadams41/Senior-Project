#ifndef UDP
#define UDP

#include <stdint-gcc.h>
#include "net/ip/ipv4.h"

typedef uint16_t udp_port;

typedef struct {
	udp_port source_port;
        udp_port dest_port;
	uint16_t length; //length of udp header + data
	uint16_t checksum; //optional in ipv4
} __attribute__ ((packed)) udp_header;

int create_udp_packet(ipv4_addr source, udp_port source_port, ipv4_addr dest, udp_port dest_port, uint8_t *data, uint32_t data_len, uint8_t **packet_return);

/* common UDP ports */
#define UDP_PORT_DNS 53
#define UDP_PORT_DHCPSERVER 67
#define UDP_PORT_DHCPCLIENT 68


#endif
