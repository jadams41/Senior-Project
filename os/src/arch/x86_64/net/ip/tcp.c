#include <stdint-gcc.h>
#include "drivers/memory/memoryManager.h"
#include "drivers/net/ethernet/realtek/8139too.h"
#include "net/ip/checksum.h"
#include "net/ip/ipv4.h"
#include "net/ip/tcp.h"
#include "types/process.h"
#include "utils/byte_order.h"
#include "utils/printk.h"
#include "utils/random.h"

static void copy_received_data_into_buffer(tcp_connection *conn, tcp_header *new_tcp_head, uint64_t tcp_len);
static void print_flags(tcp_flags f);
static int flags_equal(tcp_flags f1, tcp_flags f2);
static uint8_t get_data_offset(tcp_header *head);
static tcp_flags get_flags(tcp_header *head);
static uint16_t get_flags_raw(tcp_header *head);
static int set_data_offset_and_flags(tcp_header *head, uint8_t data_offset, tcp_flags flags);
static int create_tcp_segment(tcp_connection *conn, uint8_t data_offset, tcp_flags flags, uint16_t urgent_ptr, uint8_t *data, uint32_t data_len, uint8_t **packet_return);

static TCP_state tcp_state;

extern PROC_context *curProc;

/* todo move to a file specifically for ports */
static uint16_t get_random_dynamic_port(){
	uint16_t rand = rand_u16();
	printk_debug("random u16 = %u\n", rand);
	
	rand = rand % (LAST_DYNAMIC_PORT - FIRST_DYNAMIC_PORT + 1);
	rand = rand + FIRST_DYNAMIC_PORT;

	printk_debug("random port = %u\n", rand);
	
	return rand;
}

static seq_number get_random_initial_seq_num(){
	uint32_t rand = rand_u32();

	printk_debug("random u32 = %u\n", rand);
	
	//start after 1000 to be safe (weird behavior when I set to 0 earlier)
	rand = rand % ((sizeof(uint32_t) * 0xFF) - 1000) + 1000;

	printk_debug("random seq num = %u\n", rand);
	
	return rand;
}

/* initialize tcp_state to keep track of open connections */
void init_tcp_state(){
	tcp_state.next_connection_uid = 1234; //start here, increment everytime this number is assigned to a connection
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

	asm("CLI");
	uint64_t tcp_len = ntohs(ip_head->total_len) - sizeof(ipv4_header);
	printk_debug("ip_head->total_len = %u, sizeof(ipv4_header) = %u, tcp_len = %u\n", ntohs(ip_head->total_len), sizeof(ipv4_header), tcp_len);
	
	uint8_t *tcp_begin = (uint8_t*)ip_head;
	tcp_begin = tcp_begin + sizeof(ipv4_header);
	tcp_header *tcp_head = (tcp_header*)tcp_begin;

        uint16_t src_port = ntohs(tcp_head->src_port);
	uint16_t dest_port = ntohs(tcp_head->dest_port);
	uint16_t raw_flags = get_flags_raw(tcp_head);
	ack_number recvd_ack_num = ntohl(tcp_head->ack_num);
	
	tcp_connection *cur = tcp_state.connections;
	while(cur){
		printk_debug("connection %lu's cur_state = %d\n", cur->connection_uid, cur->cur_state);
		/* check if the received packet is associated with any open tcp connections */
		if(dest_port == cur->host_port && src_port == cur->remote_port){
			if(recvd_ack_num < cur->cur_host_seq){
				printk_warn("received ack num = %u (raw=%u) when highest sent was %u, TODO add logic to resend\n", tcp_head->ack_num, recvd_ack_num, cur->cur_host_seq);
			}
			

			switch(cur->cur_state){
			case TCP_STATE_CLOSE:
				printk_warn("I don't think we should ever be in STATE_CLOSE at this point\n");
				break;
			case TCP_STATE_SYNSENT:
				/* waiting for SYNACK from server */
				if(!(raw_flags & (TCP_FLAGMASK_SYN | TCP_FLAGMASK_ACK))){
					break;
				}
				copy_received_data_into_buffer(cur, tcp_head, tcp_len);

				cur->cur_state = TCP_STATE_ESTABLISHED;
				
				//unblock all waiting connections
				if(tcp_state.blocked.head)
					PROC_unblock_all(&tcp_state.blocked);

				break;
			case TCP_STATE_ESTABLISHED:
				/* waiting for ACKS */
				if(!(raw_flags & TCP_FLAGMASK_ACK)){
					break;
				}
				copy_received_data_into_buffer(cur, tcp_head, tcp_len);
				//unblock all waiting connections
				if(tcp_state.blocked.head)
					PROC_unblock_all(&tcp_state.blocked);

				break;
			case TCP_STATE_LASTACK:
				/* waiting for final ack */
				if(!(raw_flags & TCP_FLAGMASK_ACK)){
					break;
				}
				copy_received_data_into_buffer(cur, tcp_head, tcp_len);
				//unblock all waiting connections
				if(tcp_state.blocked.head)
					PROC_unblock_all(&tcp_state.blocked);
				
				break;
			default:
				printk("no logic to handle state %d\n", cur->cur_state);
				break;
			}
			
			//unblock all waiting connections
			if(tcp_state.blocked.head)
				PROC_unblock_all(&tcp_state.blocked);
			asm("STI");
			return;
		}
		
		cur = cur->next;
	}
	asm("STI");
	
}

static tcp_connection *open_new_tcp_connection(ipv4_addr host, ipv4_addr remote, uint16_t host_port, uint16_t remote_port, uint16_t window_size){
	/* create tcp_connection */
	tcp_connection *new_connection = (tcp_connection*)kmalloc(sizeof(tcp_connection));

	asm("CLI");
	/* initialize created tcp_connection */
	new_connection->connection_uid = tcp_state.next_connection_uid++;
	new_connection->cur_state = TCP_STATE_CLOSE;
	new_connection->host_addr = host;
	new_connection->remote_addr = remote;
	new_connection->host_port = host_port;
	new_connection->remote_port = remote_port;
	new_connection->window_size = window_size;
	new_connection->new_data_avail = 0;

	/* zero out received buffer just to be safe */
	int i;
	for(i = 0; i < IPV4_DATA_LEN; i++){
		new_connection->received[i] = 0;
	}
	new_connection->received_top = new_connection->received;
	
	new_connection->first_host_seq = new_connection->cur_host_seq = get_random_initial_seq_num();
	new_connection->first_remote_seq = new_connection->highest_remote_seq = 0;

	new_connection->fin_seen = 0;
	new_connection->psh_seen = 0;

	/* add new tcp_connection to the list of current tcp connections */
	new_connection->next = tcp_state.connections;
	tcp_state.connections = new_connection;
	tcp_state.num_connections += 1;
	
	asm("STI");

	return new_connection;
}

static int close_tcp_connection(tcp_connection *conn_to_remove){
	/* turn off interupts while we do this to keep tcp_state's connection list safe */
	asm("CLI");

	tcp_connection *cur, *last = NULL;
	if((cur = tcp_state.connections) == NULL || tcp_state.num_connections == 0){
		printk_err("Attempted to close a connection but tcp_state reports no current current connections...\n");
		asm("hlt");
	}

	while(cur){
		if(cur->connection_uid == conn_to_remove->connection_uid){
			uint64_t closed_uid = conn_to_remove->connection_uid;
			if(last == NULL){
				tcp_state.connections = cur->next;
			}
			else {
				last->next = cur->next;
			}
			
			tcp_state.num_connections -= 1;
			kfree(conn_to_remove);

			printk_debug("successfully closed tcp_connection %lu (%d connections still open)\n", closed_uid, tcp_state.num_connections);
			asm("STI");

			return 0;
		}
		last = cur;
		cur = cur->next;
	}

	printk_err("Attempted to close a connection that is not present in tcp_state's current connections...\n");
	asm("STI");
	return 1;
}

static void block_until_tcp_received(tcp_connection *conn, int ints_back_on){
	asm("CLI");
	while(conn->new_data_avail == 0){
		PROC_block_on(&tcp_state.blocked, 0);
		asm("CLI");
	}
	
	/* printk("received TCP response:\n"); */
	/* printk("\tseq_num: %u (switched=%u)\n", tcp_head->seq_num, ntohl(tcp_head->seq_num)); */
	/* printk("\tack_num: %u (switched=%u)\n", tcp_head->ack_num, ntohl(tcp_head->ack_num)); */
	/* printk("\t"); print_flags(recvd_flags); */
	/* printk("\n"); */

	if(ints_back_on)
		asm("STI");
}

/* static void block_until_specific_tcp_received(tcp_connection *conn, uint16_t expected_flags, ack_number expected_ack, int ints_back_on){ */
/* 	tcp_header *tcp_head = NULL; */
/* 	tcp_flags recvd_flags = {0,0,0,0,0,0,0,0,0}; */
/* 	uint16_t recvd_flags_raw = 0; */
/* 	ack_number recvd_ack = 0; */

/* 	conn->expected_flags = expected_flags; */
	
/* 	//interrupts off after unblocked so that we can verify without risk of being interrupted */
/* 	block_until_tcp_received(conn, 0); */

/* 	printk("received TCP response:\n"); */
/* 	printk("\tseq_num: %u (switched=%u)\n", tcp_head->seq_num, ntohl(tcp_head->seq_num)); */
/* 	printk("\tack_num: %u (switched=%u)\n", tcp_head->ack_num, ntohl(tcp_head->ack_num)); */
/* 	printk("\t"); print_flags(recvd_flags); */
/* 	printk("\n"); */
	
/* 	if(ints_back_on){ */
/* 		asm("STI"); */
/* 	} */
/* } */

static void wait_for_synack(tcp_connection *conn){
	/* expecting ACK-SYN from server */
        //uint16_t expected_resp_flags = TCP_FLAGMASK_SYN | TCP_FLAGMASK_ACK;

        /* block until we get an ACK-SYN from the server */
	while(conn->cur_state != TCP_STATE_ESTABLISHED){
		block_until_tcp_received(conn, 1);
	}

	/* ACK-SYN received */
	conn->new_data_avail = 0;
	/* conn->cur_state = TCP_STATE_ESTABLISHED; */
	
	/* /\* update next segment number to received ACK# *\/ */
	/* tcp_header *tcp_head = (tcp_header*)(conn->received); */
	/* conn->cur_host_seq = ntohl(tcp_head->ack_num); */
	/* conn->highest_remote_seq = ntohl(tcp_head->seq_num); */

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

	printk_info("sending syn to server\n");
	conn->cur_state = TCP_STATE_SYNSENT;
	/* send SYN request to server, move to STATE_SYNSENT */
	tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
	rtl8139_transmit_packet(tcp_frame, tcp_frame_len);


	printk_info("waiting for synack\n");
	/* wait for synack, rx_networking_thread will move us to STATE_ESTABLISHED once received */
	wait_for_synack(conn);
	
	/* sending ACK back to server */
	flags.syn = 0;
	flags.ack = 1;

	printk_info("acking the received synack\n");
	tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
	rtl8139_transmit_packet(tcp_frame, tcp_frame_len);
	printk_info("connection established\n");
	
	//conn->cur_host_seq += 1; //todo figure out if this is universally accurate
}


static void copy_received_data_into_buffer(tcp_connection *conn, tcp_header *new_tcp_head, uint64_t tcp_len){
	tcp_flags recvd_flags = get_flags(new_tcp_head);
	uint8_t data_offset = get_data_offset(new_tcp_head);
	ack_number recvd_ack = ntohl(new_tcp_head->ack_num); //update current sequence number to last received ack (no matter what it is)
	seq_number recvd_seq = ntohl(new_tcp_head->seq_num);

	uint64_t data_len = tcp_len - (data_offset * 4);

	printk("about to copy %lu bytes into the buffer\n", data_len);
	
	/* update the sequence and ack numbers */
	if(recvd_flags.ack){
		conn->cur_host_seq = recvd_ack; //update seq num to ack num received from server no matter what
	}
	
	/* let tcp connection thread know that we have received a response */
	conn->new_data_avail = 1;
	
	if((recvd_seq + data_len) <= conn->highest_remote_seq){
		printk_warn("already received all of this data, doing nothing\n");
		printk_warn("recvd_seq = %lu, data_len = %lu, conn->highest_remote_seq = %lu", recvd_seq, data_len, conn->highest_remote_seq);
	}
	else {
		conn->highest_remote_seq = recvd_seq > conn->highest_remote_seq ? recvd_seq : conn->highest_remote_seq;
		if(data_len > 0){
			uint8_t *data_begin = ((uint8_t*)new_tcp_head) + (data_offset * 4);
			uint8_t *new_data_begin = data_begin + (conn->highest_remote_seq - recvd_seq);
			
			while((new_data_begin < (data_begin + data_len - 1)) && (conn->received_top < (conn->received + IPV4_DATA_LEN - 1))){
				*(conn->received_top++) = *(new_data_begin++);
				// conn->new_data_avail += 1;
				conn->highest_remote_seq += 1;
			}
		        
		}
	}
	
	if(recvd_flags.fin){
	        conn->fin_seen = 1;
	}
	if(recvd_flags.psh){
	        conn->psh_seen = 1;
	}
	
}

static int tcp_wait_for_data_and_handle(tcp_connection *conn){
	uint64_t total_data_received = 0;
	
	uint8_t *tcp_frame;
	int tcp_frame_len;
	tcp_flags flags = {.ns = 0, .cwr = 0, .ece = 0, .urg = 0, .ack = 1, .psh = 0, .rst = 0, .fin = 0};
	
	/* receive data until fin seen */
	while(1){
		/* wait for data from remote */
		/* block_until_specific_tcp_received(conn, TCP_FLAGMASK_ACK, conn->cur_host_seq, 0); */
		block_until_tcp_received(conn, 1);
		
		/* handle buffered data */
		if(conn->new_data_avail){
			uint8_t *data_walker = conn->received;
			while(data_walker < conn->received_top){
				printk("%c", *(data_walker++));
				total_data_received += 1;
			}
			
			//reset the buffer
			conn->received_top = conn->received;
			conn->new_data_avail = 0;

			/* todo might need to add logic in the case that there is no data sent but fin is received */
			if(conn->fin_seen){
				conn->cur_state = TCP_STATE_CLOSEWAIT;
				asm("STI");
				break;
			}

			asm("STI");		
			/* ack the received data */
			tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
			rtl8139_transmit_packet(tcp_frame, tcp_frame_len);
		}

		if(conn->fin_seen){
			conn->cur_state = TCP_STATE_CLOSEWAIT;
			asm("STI");
			break;
		}
	}

        return total_data_received;
}

static int tcp_perform_term_handshake(tcp_connection *conn){
	uint8_t *tcp_frame;
	int tcp_frame_len;
	tcp_flags flags = {.ns = 0, .cwr = 0, .ece = 0, .urg = 0, .ack = 1, .psh = 0, .rst = 0, .fin = 0};

	/* case: we are initiating the tcp connection termination handshake */
	if(conn->cur_state == TCP_STATE_ESTABLISHED){
		//send fin

		//goto STATE_FINWAIT1, wait for ack of sent fin

		//receive ack of sent fin, goto STATE_FINWAIT2, wait for fin

		//receive fin, goto STATE_TIMEWAIT

		printk_err("not implemented yet\n");
		asm("hlt");
	}
	/* case: other device has initiated the the tcp connection termination handshake */
	else if(conn->cur_state == TCP_STATE_CLOSEWAIT){
		//send ack, goto STATE_CLOSEWAIT
		/* tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame); */
		/* rtl8139_transmit_packet(tcp_frame, tcp_frame_len); */
		/* conn->cur_state = TCP_STATE_CLOSEWAIT; */
		
		/* //send fin, goto STATE_LASTACK */
		/* flags.ack = 0; */
		/* flags.fin = 1; */
	        /* tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame); */
		/* rtl8139_transmit_packet(tcp_frame, tcp_frame_len); */
		/* conn->cur_state = TCP_STATE_LASTACK; */

		/* send ACK */
		conn->highest_remote_seq += 1;
		
	        tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
		rtl8139_transmit_packet(tcp_frame, tcp_frame_len);
		conn->cur_state = TCP_STATE_LASTACK;

		/* send FINACK */
		flags.fin = 1;
	        tcp_frame_len = create_tcp_segment(conn, (sizeof(tcp_header) /  4), flags, 0, NULL, 0, &tcp_frame);
		rtl8139_transmit_packet(tcp_frame, tcp_frame_len);
		conn->cur_state = TCP_STATE_LASTACK;
		
		/* wait for ack of sent fin, close connection when received */
		/* block_until_specific_tcp_received(conn, TCP_FLAGMASK_ACK, conn->cur_host_seq + 1, 0); */
		block_until_tcp_received(conn, 1);
		conn->cur_state = TCP_STATE_CLOSE;

		asm("STI");
		
	}
	else {
		printk_err("you are currently in state = %d, doesn't make sense for performing tcp handshake\n", conn->cur_state);
		asm("hlt");
	}
	

	return 0;
}

/*takes params:
 * ipv4_addr src
 * ipv4_addr dest
 * uint16_t src_port
 * uint16_t dest_port */
void establish_tcp_connection(void *params){
	printk_info("establish tcp pid = %u\n", curProc->pid);

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
	if((src_port = (uint16_t)paramArr[3]) == 0){
		//get random port
		src_port = get_random_dynamic_port();
	}
	dest_port = (uint16_t)paramArr[4];

	uint16_t win_size = IPV4_DATA_LEN - sizeof(tcp_header) - 20; //don't want more than one packet coming in at a time

	tcp_connection *connection = open_new_tcp_connection(src, dest, src_port, dest_port, win_size);

	//attempt to establish connection
	perform_tcp_handshake(connection);

	//receive data until fin is received
        tcp_wait_for_data_and_handle(connection);

	//send ACK of received FIN, send our own FIN, and then wait for final ACK of FIN
	tcp_perform_term_handshake(connection);

	close_tcp_connection(connection);

	kexit();
}

static void __attribute__((unused)) print_flags(tcp_flags f){
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

/* todo fix this, its so gross 
 * returns 1 if all expected flags set otherwise 0 */
static int __attribute__((unused)) flags_set(tcp_flags flags, uint16_t expected_set_flags){
	if(expected_set_flags & TCP_FLAGMASK_NS && !flags.ns){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_CWR && !flags.cwr){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_ECE && !flags.ece){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_URG && !flags.urg){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_ACK && !flags.ack){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_PSH && !flags.psh){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_RST && !flags.rst){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_SYN && !flags.syn){
		return 0;
	}
	if(expected_set_flags & TCP_FLAGMASK_FIN && !flags.fin){
		return 0;
	}

	return 1;
}

/* returns 0 if not equal and 1 if equal */
static int __attribute__((unused)) flags_equal(tcp_flags f1, tcp_flags f2){
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

static uint16_t get_flags_raw(tcp_header *head){
	uint16_t dataoff_and_flags = ntohs(head->data_offset_and_flags);
        return dataoff_and_flags & TCP_FLAGMASK_ALL;
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
	header->ack_num = htonl(conn->highest_remote_seq + 1);

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
