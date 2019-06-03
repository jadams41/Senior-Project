#ifndef _TCP_H
#define _TCP_H

#include <stdint-gcc.h>

typedef struct tcp_header tcp_header;
struct tcp_header {
	uint16_t src_port;  /* Source Port - identifies the sending port */
	uint16_t dest_port; /* Destination Port - identifies the receiving port */
	uint32_t seq_num;   /* Sequence Number - if SYN set to 1, then initial sequence #. if SYN == 0, accumulated seq num. */
	uint32_t ack_num;   /* Acknowledgement Number - if ACK flag set, indicates next seq# that sender of ACK is expecting. */
	uint16_t data_offset_and_flags; /* Data Offset - size of tcp header in 32 bit words (first 4 bits). Reserved (next 3 bits, = 0). Flags (last 9 bits). */
	uint16_t win_size;   /* Window Size - size of tthe receive window (# bytes sender of segment is currently willing to receive */
	uint16_t checksum;   /* Checksum - Calculated over Payload and Pseudo-Header (Source IP, Dest IP, Protocol # for TCP, length of TCP-Headers including payload) */
	uint16_t urgent_ptr; /* Urgent Pointer - If URG flag is set, then this field is an offset from the sequence number indicating the last urgent data byte */ 

	/* optional extension field, Options (0-320 bits, divisible by 32)
	 * present if data offset > 5.
	 * Padded at the end with "0" bytes if necessary */

	/* Data */
}__attribute__((packed));

/* TCP pseudo header used for calculating checksum */
typedef struct tcp_pseudo_header tcp_pseudo_header;
struct tcp_pseudo_header {
	uint32_t source_ip;   //source ipv4 address
	uint32_t dest_ip;     //destination ipv4 address
	uint8_t  reserved;    //zero out
	uint8_t  protocol;    //IP protocol (will always be TCP [0x06])
	uint16_t tcp_seg_len; //computed length of the TCP segment (header and data)
}__attribute__((packed));


/* TCP flags (network order in packet) */
typedef struct tcp_flags tcp_flags;
struct tcp_flags {
	uint8_t ns:1; /* ECN-nonce - concealment protection (experimental) */
	uint8_t cwr:1; /* Congestion Window Reduced - Set by sending host to indicate that it received a TCP segment w/ ECE flag set and had responded in congestion control mechanism */
	uint8_t ece:1; /* ECN-Echo - if SYN=1, indicates TCP peer is ECN capable. if SYN=0, packet w/ congestion experience flag set was received during normal transmission */
	uint8_t urg:1; /* Urgent - Indicates that Urgent pointer field is significant */
	uint8_t ack:1; /* Acknowledgment - Indicates that `ack_num` field is significant. All packets after initial SYN packet sent by client should have this flag set */
	uint8_t psh:1; /* Push Function - Asks to push the buffered data to the receiving application */
	uint8_t rst:1; /* Reset - Reset the connection */
	uint8_t syn:1; /* Syunchronize Seq Nums - Only firstt packet sent from each end should have this flag set. */
	uint8_t fin:1; /* Finish - Last packet from sender */
}__attribute__((packed));

/* TCP client states (in order */
enum tcp_client_states {
	TCP_STATE_CLOSE       = 0, //No connection state at all
	TCP_STATE_SYNSENT     = 1, //Waiting for a matching connection request after having sent a connection request.
	TCP_STATE_ESTABLISHED = 2, //Open connection, data received can be delivered to the user (normal state for data transfer phase of connection.
	TCP_STATE_FINWAIT1    = 3, //Waiting for a connection termination request from the remote TCP, or ack of connection termination request previously set.
	TCP_STATE_FINWAIT2    = 4, //Waiting for a connection termination request from the remote TCP.
	TCP_STATE_CLOSEWAIT   = 5, //Waiting for a connection termination request from the local user.
	TCP_STATE_CLOSING     = 6, //Waiting for a connection termination acknowledgment from remote TCP
	TCP_STATE_LASTACK     = 7, //Waiting for acknowledgment of the connection termination request previously sent to the remote TCP.
	TCP_STATE_TIMEWAIT    = 8, //Waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request
};

//todo remove (just a testing function to see if we can send an initial syn and receive an appropriate response)
int test_tcp_syn();

/* TCP Connection Establishment flow overview:
   1) [SYN] (Client->Server)
      - Client attempts to connect with server.
      - Client informs Server of its initial sequence number.
      * `seq_num` = <client_initial_seq_num>
   2) [SYNACK] (Server->Client)
      - Server acknowledges Client's synchronization request.
      - `ack_num` field informs Client of the next sequence number Server expects
      * `seq_num` = <server_initial_seq_num> 
      * `ack_num` = <next seq_num server expects from client>
   3) [ACK] (Client->Server)
      - Client acknowledes Server's SYNACK.
      * `seq_num` = <client_initial_seq + 1>
      * `ack_num` = <server_initial_seq + 1>

 * Connection Established! */
int establish_tcp_connection(ipv4_addr src, ipv4_addr dest, uint16_t src_port, uint16_t dest_port);

#endif
