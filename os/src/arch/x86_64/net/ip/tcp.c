#include <stdint-gcc.h>
#include "drivers/net/ethernet/realtek/8139too.h"
#include "net/ip/checksum.h"
#include "net/ip/ipv4.h"
#include "net/ip/tcp.h"
#include "types/process.h"
#include "utils/byte_order.h"
#include "utils/printk.h"
#include "drivers/memory/memoryManager.h"

static void print_flags(tcp_flags f);
static int flags_equal(tcp_flags f1, tcp_flags f2);
static uint8_t get_data_offset(tcp_header *head);
static tcp_flags get_flags(tcp_header *head);
static int set_data_offset_and_flags(tcp_header *head, uint8_t data_offset, tcp_flags flags);
static int create_tcp_segment(tcp_connection *conn, uint8_t data_offset, tcp_flags flags, uint16_t urgent_ptr, uint8_t *data, uint32_t data_len, uint8_t **packet_return);

static TCP_state tcp_state;

/* initialize tcp_state to keep track of open connections */
void init_tcp_state(){
	tcp_state.num_connections = 0;
	tcp_state.connections = NULL;
	PROC_init_queue(&tcp_state.blocked);
}

/* when tcp segment is received, extract port number and see if it matches any open connections */
void handle_received_tcp_segment(ipv4_header *ip_head){
	if(tcp_state.num_connections == 0){
		printk_warn("received TCP transmission but there are no open connections, discarding\n");
		return;
	}
	if(tcp_state.connections == NULL){
		printk_err("something broke along the way... apparently %d connections, but connection list is NULL\n", tcp_state.num_connections);
		asm("hlt");
	}

	uint64_t tcp_len = ntohs(ip_head->total_len) - sizeof(ipv4_header);

	tcp_header *tcp_head = (tcp_header*)(((uint8_t*)ip_head) + sizeof(ipv4_header));
	//uint16_t src_port = ntohs(tcp_head->src_port);
	uint16_t dest_port = ntohs(tcp_head->dest_port);
	
	tcp_connection *cur = tcp_state.connections;
	while(cur){
		/* check if the received packet is associated with any open tcp connections */
		if(dest_port == cur->host_port){
			uint8_t *tcp_seg_begin = (uint8_t*)tcp_head;
			
			int i;
			for(i = 0; i < tcp_len; i++){
				cur->received[i] = tcp_seg_begin[i];
			}
			cur->new_data_avail = tcp_len;

			//unblock all waiting connections
			PROC_unblock_all(&tcp_state.blocked);
			return;
		}
		
		cur = cur->next;
	}

	
}

static void block_until_tcp_received(tcp_connection *conn, int ints_back_on){
	asm("CLI");
	while(conn->new_data_avail == 0){
		PROC_block_on(&tcp_state.blocked, 1);
		asm("CLI");
	}
	
	if(ints_back_on)
		asm("STI");
}

static void block_until_specific_tcp_received(tcp_connection *conn, tcp_flags expected_flags, ack_number expected_ack, int ints_back_on){
	tcp_header *tcp_head = NULL;
	tcp_flags recvd_flags = {0,0,0,0,0,0,0,0,0};
	ack_number recvd_ack = 0;
	
	while(1){
		//interrupts off after unblocked so that we can verify without risk of being interrupted
		block_until_tcp_received(conn, 0);
		tcp_head = (tcp_header*)(conn->received);

		recvd_ack = ntohl(tcp_head->ack_num);
		recvd_flags = get_flags(tcp_head);

		//todo add more handling code here for error cases (dropped packets, wrong ack, etc)
		if(flags_equal(expected_flags, recvd_flags) && expected_ack == recvd_ack)
			break;
	}

	printk("received TCP response:\n");
	printk("\tseq_num: %u (switched=%u)\n", tcp_head->seq_num, ntohl(tcp_head->seq_num));
	printk("\tack_num: %u (switched=%u)\n", tcp_head->ack_num, ntohl(tcp_head->ack_num));
	printk("\t"); print_flags(recvd_flags);
	printk("\n");
	
	if(ints_back_on){
		asm("STI");
	}
}

static void wait_for_synack(tcp_connection *conn){
	/* expecting ACK-SYN from server */
	tcp_flags expected_resp_flags = {.ns = 0, .cwr = 0, .ece = 0, .urg = 0, .ack = 1, .psh = 0, .rst = 0, .syn = 1, .fin = 0};

        /* block until we get an ACK-SYN from the server */
	block_until_specific_tcp_received(conn, expected_resp_flags, conn->cur_host_seq + 1, 0);

	/* update next segment number to received ACK# */
	tcp_header *tcp_head = (tcp_header*)(conn->received);
	conn->cur_host_seq = ntohl(tcp_head->ack_num);
	conn->last_remote_seq = ntohl(tcp_head->seq_num) + 1;
	
	asm("STI");
}

void test_tcp_syn(){
	ipv4_addr src = str_to_ipv4(STATIC_IP); //static ip of this machine
	ipv4_addr dest = str_to_ipv4("172.16.210.183");   //random, ripped from wireshark capture
	uint16_t src_port = 57746; //random, ripped from wireshark capture
	uint16_t dest_port = 2023; //random, ripped from wireshark capture

	uint64_t *proc_params = (uint64_t*)kmalloc(sizeof(uint64_t) * 5);
	proc_params[0] = 5;
	proc_params[1] = src;
	proc_params[2] = dest;
	proc_params[3] = src_port;
	proc_params[4] = dest_port;

	PROC_create_kthread(establish_tcp_connection, (void*)proc_params);
}

void tcp_connect(ipv4_addr src, ipv4_addr dest, uint16_t src_port, uint16_t dest_port){
	uint64_t *proc_params = (uint64_t*)kmalloc(sizeof(uint64_t) * 5);
	proc_params[0] = 5;
	proc_params[1] = src;
	proc_params[2] = dest;
	proc_params[3] = src_port;
	proc_params[4] = dest_port;

	PROC_create_kthread(establish_tcp_connection, (void*)proc_params);
}

/* Handshake procedure overview:
 * 1) sends syn to server
 *    - sends initial host sequence number
 * 2) waits for synack from server
 *    - upon synack receive, stores sequence # supplied by remote
 * 3) acknowledges connection with server
 *    - ack_num = (initial_remote_seq_num) + 1
 *    - seq_num = (initial_host_seq_num) + 1
 * Connection considered to be established as soon as connection ACK is sent
*/
static void perform_tcp_handshake(tcp_connection *conn){
	uint8_t *tcp_frame;
	int tcp_frame_len;
	tcp_flags flags = {.ns = 0, .cwr = 0, .ece = 0, .urg = 0, .ack = 0, .psh = 0, .rst = 0, .syn = 1, .fin = 0};
	
	//send SYN request to server
	tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
	rtl8139_transmit_packet(tcp_frame, tcp_frame_len);

	wait_for_synack(conn);

	/* ACK-SYN received */
	conn->new_data_avail = 0;
	
	/* sending ACK back to server */
	flags.syn = 0;
	flags.ack = 1;
	
	tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
	rtl8139_transmit_packet(tcp_frame, tcp_frame_len);
	/* connection should now be established */
	conn->cur_state = TCP_STATE_ESTABLISHED;
}

/* returns 0 if everything received correctly,
 * returns 1 if received FIN from sender
 * returns 2 if data is bad */
static int handle_received_tcp_data(tcp_connection *conn){
	if(conn->new_data_avail == 0){
		printk_err("dafuq\n");
		return 2;
	}

	tcp_header *head = (tcp_header*)(conn->received);

	tcp_flags recvd_flags = get_flags(head);
	uint8_t data_offset = get_data_offset(head);
	uint8_t *data_begin = conn->received + (data_offset * 4);
	uint64_t data_len = conn->new_data_avail - (data_offset * 4);

	/* update the sequence and ack numbers */
	
	
	/* todo replace with something more useful... just printing data right now */
	printk("received %u bytes of data in sequence #%u\n", data_len, head->seq_num);
	if(data_len > 0){
		printk("-------------------- data --------------------\n");
		int i;
		for(i = 0; i < data_len; i++){
			printk("%c", data_begin[i]);
		}
		printk("\n---------------------------------------------\n");
	}
	if(recvd_flags.fin){
		return 1;
	}

	return 0;
	
}

static int __attribute__((unused)) tcp_receive_data(tcp_connection *conn){
	uint8_t *tcp_frame;
	int tcp_frame_len;
	tcp_flags flags = {.ns = 0, .cwr = 0, .ece = 0, .urg = 0, .ack = 1, .psh = 0, .rst = 0, .syn = 0, .fin = 0};

	/* receive data until fin seen */
	while(1){
		/* wait for data from remote */
		block_until_specific_tcp_received(conn, flags, conn->cur_host_seq + 1, 0);

		/* handle received data */
		handle_received_tcp_data(conn);
		conn->new_data_avail = 0;

		/* ack the received data */
		tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
		rtl8139_transmit_packet(tcp_frame, tcp_frame_len);
		
	}

	return 0;
}

/*takes params:
 * ipv4_addr src
 * ipv4_addr dest
 * uint16_t src_port
 * uint16_t dest_port */
void establish_tcp_connection(void *params){
	/* pull out k_thread params */
	uint64_t *paramArr = (uint64_t*)params;
	if(params == 0){
		printk_err("No parameters were passed to establish_tcp_connection, aborting\n");
		return;
	}

	uint64_t paramLen = *paramArr;
	if(paramLen != 5){
		printk_err("wrong number of parameters were passed to establish_tcp_connection (received %d, expected %d), aborting\n",
			   paramLen, 5);
		return;
	}
	
	/* set up initial tcp SYN */
	ipv4_addr src, dest;
	uint16_t src_port, dest_port;
	src = (ipv4_addr)paramArr[1];
	dest = (ipv4_addr)paramArr[2];
	src_port = (uint16_t)paramArr[3];
	dest_port = (uint16_t)paramArr[4];

	uint16_t win_size = IPV4_DATA_LEN - sizeof(tcp_header) - 20; //don't want more than one packet coming in at a time

	/* create tcp_connection in anticipation for sending connection request */
	tcp_connection *new_connection = (tcp_connection*)kmalloc(sizeof(tcp_connection));
	new_connection->cur_state = TCP_STATE_SYNSENT;
	new_connection->host_addr = src;
	new_connection->remote_addr = dest;
	new_connection->host_port = src_port;
	new_connection->window_size = win_size;
	new_connection->remote_port = dest_port;
	new_connection->new_data_avail = 0;

	int i;
	for(i = 0; i < IPV4_DATA_LEN; i++){
		new_connection->received[i] = 0;
	}

	new_connection->cur_host_seq = 5489; //todo replace with random number
	new_connection->last_remote_seq = 0; //doesn't mean anything yet, will be updated when first segment from server is received
	
	new_connection->next = NULL;

	asm("CLI");
        //add new_connection to list of tcp_connections
	if(tcp_state.connections == NULL){
		tcp_state.connections = new_connection;
		tcp_state.num_connections = 1;
	}
	else {
		new_connection->next = tcp_state.connections;
		tcp_state.connections = new_connection;
		tcp_state.num_connections += 1;
	}
	asm("STI");
	
	perform_tcp_handshake(new_connection);

	//tcp_receive_data(new_connection);
}

static void print_flags(tcp_flags f){
	int flags_seen = 0;

	printk("Flags: ");
	if(f.ns && ++flags_seen){
		printk("NS ");
	}
	if(f.cwr && ++flags_seen){
		printk("CWR ");
	}
	if(f.ece && ++flags_seen){
		printk("ECE ");
	}
	if(f.urg && ++flags_seen){
		printk("URG ");
	}
	if(f.ack && ++flags_seen){
		printk("ACK ");
	}
	if(f.psh && ++flags_seen){
		printk("PSH ");
	}
	if(f.rst && ++flags_seen){
		printk("RST ");
	}
	if(f.syn && ++flags_seen){
		printk("SYN ");
	}
	if(f.fin && ++flags_seen){
		printk("FIN ");
	}
	if(flags_seen == 0){
		printk("NONE ");
	}
	
	printk("\n");
	
}

/* returns 0 if not equal and 1 if equal */
static int flags_equal(tcp_flags f1, tcp_flags f2){
	if(f1.ns != f2.ns)
		return 0;
	if(f1.cwr != f2.cwr)
		return 0;
	if(f1.ece != f2.ece)
		return 0;
	if(f1.urg != f2.urg)
		return 0;
	if(f1.ack != f2.ack)
		return 0;
	if(f1.psh != f2.psh)
		return 0;
	if(f1.rst != f2.rst)
		return 0;
	if(f1.syn != f2.syn)
		return 0;
	if(f1.fin != f2.fin)
		return 0;

	return 1;
	
}

static uint8_t get_data_offset(tcp_header *head){
	uint16_t dataoff_mask = 0b1111000000000000;
	uint16_t dataoff_and_flags = ntohs(head->data_offset_and_flags);

	uint8_t data_offset = ((dataoff_and_flags & dataoff_mask) >> 12);

	return data_offset;
}

static tcp_flags get_flags(tcp_header *head){
	tcp_flags flags = {.ns = 0, .cwr = 0, .ece = 0, .urg = 0, .ack = 0, .psh = 0, .rst = 0, .fin = 0};
	
	uint16_t dataoff_and_flags = ntohs(head->data_offset_and_flags);
	if(dataoff_and_flags & TCP_FLAGMASK_NS)
		flags.ns = 1;
	if(dataoff_and_flags & TCP_FLAGMASK_CWR)
		flags.cwr = 1;
	if(dataoff_and_flags & TCP_FLAGMASK_ECE)
		flags.ece = 1;
	if(dataoff_and_flags & TCP_FLAGMASK_URG)
		flags.urg = 1;
	if(dataoff_and_flags & TCP_FLAGMASK_ACK)
		flags.ack = 1;
	if(dataoff_and_flags & TCP_FLAGMASK_PSH)
		flags.psh = 1;
	if(dataoff_and_flags & TCP_FLAGMASK_SYN)
		flags.syn = 1;
	if(dataoff_and_flags & TCP_FLAGMASK_FIN)
		flags.fin = 1;

	return flags;
}

static int set_data_offset_and_flags(tcp_header *head, uint8_t data_offset, tcp_flags flags){
	uint16_t combined = 0;
	
	if((data_offset & 0b11110000) != 0){
		printk_err("bad data offset (field is only 4 bits)\n");
		return 1;
	}

	combined = data_offset; //put data offset in lowest 4 bits
	combined <<= 12; //move data offset into highest 4 bits (where this info is expected)

	/* reserved bits should already be correctly set to 0 */

	/* set flags in combined */
	//todo replace with more efficient logic (extra rigorous to get things working initially)
	if(flags.ns)
		combined |= TCP_FLAGMASK_NS;
	if(flags.cwr)
		combined |= TCP_FLAGMASK_CWR;
	if(flags.ece)
		combined |= TCP_FLAGMASK_ECE;
	if(flags.urg)
		combined |= TCP_FLAGMASK_URG;
	if(flags.ack)
		combined |= TCP_FLAGMASK_ACK;
	if(flags.psh)
		combined |= TCP_FLAGMASK_PSH;
	if(flags.rst)
		combined |= TCP_FLAGMASK_RST;
	if(flags.syn)
		combined |= TCP_FLAGMASK_SYN;
	if(flags.fin)
		combined |= TCP_FLAGMASK_FIN;

	printk("combined = 0x%x\n", combined);
	
	head->data_offset_and_flags = htons(combined);
	return 0;
}

static int create_tcp_segment(tcp_connection *conn, uint8_t data_offset, tcp_flags flags, uint16_t urgent_ptr, uint8_t *data, uint32_t data_len, uint8_t **packet_return){
	uint8_t packet[IPV4_DATA_LEN + sizeof(tcp_pseudo_header)];

	tcp_pseudo_header *pseudo = (tcp_pseudo_header*)packet;
	tcp_header *header = (tcp_header*)(packet + sizeof(tcp_pseudo_header));
	uint8_t *data_begin = packet + sizeof(tcp_pseudo_header) + sizeof(tcp_header);
	
	if(data_len > (IPV4_DATA_LEN - sizeof(tcp_header))){
		printk_warn("attempted to send tcp packet with too much data/n");
		*packet_return = (void*)0;
		return 0;
	}

	uint32_t total_packet_len = sizeof(tcp_header) + data_len;

	header->src_port = htons(conn->host_port);
	header->dest_port = htons(conn->remote_port);
	header->seq_num = htonl(conn->cur_host_seq);
	header->ack_num = htonl(conn->last_remote_seq);

	set_data_offset_and_flags(header, data_offset, flags);

	header->win_size = htons(conn->window_size);
	header->urgent_ptr = htons(urgent_ptr);
	header->checksum = 0; //will set later
	header->urgent_ptr = htons(urgent_ptr);

	//copy data into packet (todo replace w/ memcpy)
	int i;
	for(i = 0; i < data_len; i++){
		data_begin[i] = data[i];
	}
	
	//prepare pseudo header for calculating checksum
	pseudo->source_ip = htonl(conn->host_addr);
	pseudo->dest_ip = htonl(conn->remote_addr);
	pseudo->reserved = 0;
	pseudo->protocol = IPV4_PROTO_TCP;
	pseudo->tcp_seg_len = htons(/* sizeof(tcp_pseudo_header) + */ sizeof(tcp_header) + data_len);

	//calculate checksum over pseudo_header, tcp_header, and data
	unsigned short cksum = in_cksum((unsigned short*)packet, sizeof(tcp_pseudo_header) + total_packet_len);
	header->checksum = cksum; //htons(cksum);

	printk("data_offset and flags = 0x%x\n", header->data_offset_and_flags);
	
	return create_ipv4_packet(IPV4_PROTO_TCP, conn->host_addr, conn->remote_addr, (uint8_t*)header, total_packet_len, packet_return);
}


/* static int create_tcp_packet(ipv4_addr src, ipv4_addr dest, uint16_t src_port, uint16_t dest_port, uint32_t seq_num, uint32_t ack_num, uint8_t data_offset, tcp_flags flags, uint16_t win_size, uint16_t urgent_ptr, uint8_t *data, uint32_t data_len, uint8_t **packet_return){ */
/* 	uint8_t packet[IPV4_DATA_LEN + sizeof(tcp_pseudo_header)]; */

/* 	tcp_pseudo_header *pseudo = (tcp_pseudo_header*)packet; */
/* 	tcp_header *header = (tcp_header*)(packet + sizeof(tcp_pseudo_header)); */
/* 	uint8_t *data_begin = packet + sizeof(tcp_pseudo_header) + sizeof(tcp_header); */
	
/* 	if(data_len > (IPV4_DATA_LEN - sizeof(tcp_header))){ */
/* 		printk_warn("attempted to send tcp packet with too much data/n"); */
/* 		*packet_return = (void*)0; */
/* 		return 0; */
/* 	} */

/* 	uint32_t total_packet_len = sizeof(tcp_header) + data_len; */

/* 	header->src_port = htons(src_port); */
/* 	header->dest_port = htons(dest_port); */
/* 	header->seq_num = htonl(seq_num); */
/* 	header->ack_num = htonl(ack_num); */

/* 	set_data_offset_and_flags(header, data_offset, flags); */

/* 	header->win_size = htons(win_size); */
/* 	header->urgent_ptr = htons(urgent_ptr); */
/* 	header->checksum = 0; //will set later */
/* 	header->urgent_ptr = htons(urgent_ptr); */

/* 	//copy data into packet (todo replace w/ memcpy) */
/* 	int i; */
/* 	for(i = 0; i < data_len; i++){ */
/* 		data_begin[i] = data[i]; */
/* 	} */
	
/* 	//prepare pseudo header for calculating checksum */
/* 	pseudo->source_ip = htonl(src); */
/* 	pseudo->dest_ip = htonl(dest); */
/* 	pseudo->reserved = 0; */
/* 	pseudo->protocol = IPV4_PROTO_TCP; */
/* 	pseudo->tcp_seg_len = htons(/\* sizeof(tcp_pseudo_header) + *\/ sizeof(tcp_header) + data_len); */

/* 	//calculate checksum over pseudo_header, tcp_header, and data */
/* 	unsigned short cksum = in_cksum((unsigned short*)packet, sizeof(tcp_pseudo_header) + total_packet_len); */
/* 	header->checksum = cksum; //htons(cksum); */

/* 	printk("data_offset and flags = 0x%x\n", header->data_offset_and_flags); */
	
/* 	return create_ipv4_packet(IPV4_PROTO_TCP, src, dest, (uint8_t*)header, total_packet_len, packet_return); */
/* } */
