#ifndef IPV4
#define IPV4

#include <stdint-gcc.h>

/* typedef struct { */
/* 	uint8_t reserved:1; //must be 0 */
/* 	uint8_t df:1;       //Don't fragment - if set and fragmentation is required to route the packet, then the packet is dropped */
/* 	uint8_t mf:1;       //More Fragments - Cleared for unfragmented packets. For fragmented packets, all fragments except last have MF set. */
/* } __attribute__((packed)) ipv4_header_flags; */

/* typedef struct { */
/* 	uint8_t  version:4;  /\* Version  */
/* 				    - always 4 for ipv4 *\/ */
/* 	uint8_t  ihl:4;      /\* Internet Header Length  */
/* 				    - length of header (in 32-bit words) *\/ */
/* 	uint8_t  dscp:6;         /\* Differentiated Services Code Point  */
/* 				    - see below *\/ */
/* 	uint8_t  ecn:2;          /\* Explicit Congestion Notification  */
/* 				    - see below*\/ */
/* 	uint16_t total_len:16;   /\* Total Length  */
/* 				    - length of the entire packet (in bytes) *\/ */
/* 	uint16_t  id:16;           /\* Identification  */
/* 				    - primarily used for identifying the group  */
/* 				      of fragments of a single IP datagram *\/ */
/* 	/\* ipv4_header_flags flags:3; *\/ /\* Flags */
/* 	                                    This was breaking offsets for some reason... */
/* 					    - see above *\/ */
/* 	uint8_t flag_reserved:1; */
/* 	uint8_t flag_df:1; */
/* 	uint8_t flag_mf:1; */
	
/* 	uint16_t frag_off:13;    /\* Fragmentation Offset  */
/* 				    - Specifies the offset of a particular  */
/* 				      fragment relative to the beginning of the  */
/* 				      original unfragmented IP datagram.  */
/* 				    - The first fragment has an offset of zero. *\/ */
/* 	uint8_t  ttl:8;         /\* Time To Live */
/* 				    - Helps prevent datagrams from persisting on */
/* 				      an internet. */
/* 				    - In practice, specifies hop-count before */
/* 				      the router discards it. *\/ */
/* 	uint8_t  protocol:8;     /\* Protocol */
/* 				    - Defines the protocol used in the data */
/* 				      portion of the IP datagram. *\/ */
/* 	uint16_t header_check:16;/\* Header Checksum */
/* 				    - Used for error-checking of the header *\/ */
/* 	uint32_t source:32;      /\* Source IPv4 Address  */
/* 				    - Note: may be changed in transit by a  */
/* 				      Network Address Translation Device *\/ */
/* 	uint32_t dest:32;        /\* Destination Ipv4 Address  */
/* 				    - Note: may be changed in transit by a */
/* 				      Network Address Translation Device *\/ */
/* } __attribute__((packed)) ipv4_header; */

typedef struct {
	uint8_t   version__ihl:8;      /* Version (4-bits) & Internet Header Length (4-bits) */
	uint8_t   dscp__ecn:8;         /* Differentiated Services Code Point (6-bits) & Explicit Congestion Notification (2-bits) */
	uint16_t  total_len:16;        /* Total Length */
	uint16_t  id:16;               /* Identification */
	uint16_t  flags__frag_off:16;  /* Flags (3-bits) & Fragmentation Offset (13-bits) */
	uint8_t   ttl:8;               /* Time To Live */
	uint8_t   protocol:8;          /* Protocol */
	uint16_t  header_check:16;     /* Header Checksum */
	uint32_t  source:32;           /* Source IPv4 Address */
	uint32_t  dest:32;             /* Destination Ipv4 Address */
} __attribute__((packed)) ipv4_header;

/* Masks to extract all ipv4_header combined fields */
// VERSION__IHL
#define IPV4_PACKET_VER_MASK       0b11110000
#define IPV4_PACKET_IHL_MASK       0b00001111
// DSCP__ECN
#define IPV4_PACKET_DSCP_MASK      0b11111100
#define IPV4_PACKET_ECN_MASK       0b00000011
// FLAGS__FRAG_OFF
#define IPV4_FLAGS_RESERVED_MASK   0b1000000000000000
#define IPV4_FLAGS_DONTFRAG_MASK   0b0100000000000000
#define IPV4_FLAGS_MOREFRAG_MASK   0b0010000000000000
#define IPV4_FRAGMENTOFFSET_MASK   0b0001111111111111

void handle_received_ip_packet(uint8_t *frame, unsigned int frame_len);
int create_ipv4_packet(uint8_t protocol, uint32_t source, uint32_t dest, uint8_t *data, uint64_t data_len, uint8_t **packet_return);

#define IPV4_VERSION 4 //version is always 4 for ipv4

#define IPV4_MIN_IHL 5  //just the required fields (no options) => packet length = 20 bytes
#define MAX_IHL 15 //max options => packet length = 60 bytes

/* DSCP values (common) 
 * not standardized, but according to wikipedia, most networks use:
 *   Default Forwarding   (DF) - typically best-effort traffic
 *   Expedited Forwarding (EF) - dedicated to low-loss, low-latency traffic
 *   Assured Forwarding   (AF) - gives assurance of delivery under prescribed conditions
 *   Class Selector            - maintain backward compatibility with the IP precedence field

 * NOTE: information taken from: "https://en.wikipedia.org/wiki/Differentiated_services" */
#define IPV4_DSCP_DF 0  /* Best effort */
#define IPV4_DSCP_EF 46 /* high priority expedited forwarding (EF) */
//Assured Forwarding DSCP values 
#define IPV4_DSCP_AF11 10 //Class = 1, Drop_Prob = low,  Service Class Name = High-Throughput Data
#define IPV4_DSCP_AF12 12 //Class = 1, Drop_Prob = med,  Service Class Name = High-Throughput Data
#define IPV4_DSCP_AF13 14 //Class = 1, Drop_Prob = high, Service Class Name = High-Throughput Data
#define IPV4_DSCP_AF21 18 //Class = 2, Drop_Prob = low,  Service Class Name = Low-Latency Data
#define IPV4_DSCP_AF22 20 //Class = 2, Drop_Prob = med,  Service Class Name = Low-Latency Data
#define IPV4_DSCP_AF23 22 //Class = 2, Drop_Prob = high, Service Class Name = Low-Latency Data
#define IPV4_DSCP_AF31 26 //Class = 3, Drop_Prob = low,  Service Class Name = Multimedia Streaming
#define IPV4_DSCP_AF32 28 //Class = 3, Drop_Prob = med,  Service Class Name = Multimedia Streaming
#define IPV4_DSCP_AF33 30 //Class = 3, Drop_Prob = high, Service Class Name = Multimedia Streaming
#define IPV4_DSCP_AF41 34 //Class = 4, Drop_Prob = low,  Service Class Name = Multimedia Streaming
#define IPV4_DSCP_AF42 36 //Class = 4, Drop_Prob = med,  Service Class Name = Multimedia Streaming
#define IPV4_DSCP_AF43 38 //Class = 4, Drop_Prob = high, Service Class Name = Multimedia Streaming
//Class Selector Codes
#define IPV4_DSCP_CS0  0  //Service Class Name = Standard
#define IPV4_DSCP_CS1  8  //Service Class Name = Low-Priority Data
#define IPV4_DSCP_CS2  16 //Service Class Name = OAM (?)
#define IPV4_DSCP_CS3  24 //Service Class Name = Broadcast video
#define IPV4_DSCP_CS4  32 //Service Class Name = Real-Time Interactive
#define IPV4_DSCP_CS5  40 //Service Class Name = Signaliing
#define IPV4_DSCP_CS6  48 //Service Class Name = Network Control
#define IPV4_DSCP_CS7  56 //Service Class Name = ?

/* ECN values 
 * NOTE: information taken from https://en.wikipedia.org/wiki/Explicit_Congestion_Notification */
#define IPV4_ECN_NONECT 0b00 //Non ECN-Capable Transport, Non-ECT
#define IPV4_ECN_ECT0   0b10 //ECN-Capable Transport0 (seems interchangeable w/ ECT1)
#define IPV4_ECN_ECT1   0b01 //ECN-Capable Transport1 (seems interchangeable w/ ECT0)
#define IPV4_ECN_CE     0b11 //Congestion Encountered

/* Total Length Boundary values */
#define IPV4_TOTLEN_MIN 20    //header without data
#define IPV4_TOTLEN_MAX 65535 //max value for 2-byte field

/* Fragmentation Offset boundary values */
#define IPV4_FRAGOFF_FIRST 0             //first fragmented packet has offset 0
#define IPV4_FRAGOFF_MAX   (1 << 13) - 1 //max possible offset

/* Time To Live values */
#define IPV4_TTL_RECOMMENDED 64  //according to https://en.wikipedia.org/wiki/Time_to_live
#define IPV4_TTL_MAX         255

/* IPv4 Protocol numbers
 * Note: will add more as necessary. */
#define IPV4_PROTO_ICMP 0x01
#define IPV4_PROTO_TCP  0x06
#define IPV4_PROTO_UDP  0x11

#endif
