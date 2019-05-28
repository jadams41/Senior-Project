#include <stdint-gcc.h>
#include "ethernet.h"
#include "utils/printk.h"
#include "drivers/memory/memoryManager.h"
#include "utils/byte_order.h"

hw_addr broadcast_mac = {.bytes={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

hw_addr broadcast_mac_addr(){
	return broadcast_mac;
}

hw_addr create_hw_addr(uint8_t *hw_addr_ptr){
	hw_addr new_hw_addr;
	int i;
	
	for(i = 0; i < ETH_ALEN; i++){
		new_hw_addr.bytes[i] = hw_addr_ptr[i];
	}

	return new_hw_addr;
}

int hw_addr_compare(hw_addr addr1, hw_addr addr2){
	int i;
	for(i = 0; i < ETH_ALEN; i++){
		if(addr1.bytes[i] != addr2.bytes[i]){
			return addr1.bytes[i] - addr2.bytes[i];
		}
	}

	return 0;
}

char *hw_addr_to_str(hw_addr addr){
	char *ret_str = (char*)kmalloc(sizeof(char) * ((2 * 6) + 5 + 1));
	sprintk(ret_str, "%02X:%02X:%02X:%02X:%02X:%02X",
		addr.bytes[0], addr.bytes[1], addr.bytes[2], addr.bytes[3], addr.bytes[4], addr.bytes[5]);

	return ret_str;
}

/**
   @dest         destination mac address (should point to an array of 6 bytes in network order)
   @src          source mac address (should point to an array of 6 bytes in network order)
   @ethertype    type of frame this should be
   @data         data to put in frame's payload
   @data_len     length of the supplied payload data must be less than 1500 bytes
   @frame_return address of the newly created ethernet frame (if successful)
*/
int create_ethernet_frame(hw_addr dest, hw_addr src, uint16_t ethertype, uint8_t *data, uint64_t data_len, uint8_t **frame_return){
	uint64_t new_frame_len = 0;
	uint8_t *new_frame = NULL;
	uint8_t *new_frame_data = NULL;
	eth_frame_header *new_frame_header;

	if(data_len > 1500){
		printk_err("in order to create ethernet frame, payload data must less than 1500 bytes long (supplied data was %lu bytes)\n", data_len);
		goto out;
	}

	new_frame_len = (2 * ETH_ALEN) + sizeof(ethertype) + data_len;
	new_frame = (uint8_t*)kmalloc(new_frame_len);

	if(new_frame == NULL){
		printk_err("kmalloc failure\n");
		new_frame_len = 0;
		goto out;
	}
	
	new_frame_header = (eth_frame_header*)new_frame;
	new_frame_data = new_frame + sizeof(eth_frame_header);
	
	//copy in source and destination mac addresses
	int i;
	for(i = 0; i < 6; i++){
                //todo replace w/ memcpy
		new_frame_header->dest_mac[i] = broadcast_mac.bytes[i]; //= dest.bytes[i]; todo: CHANGE THIS BACK
		new_frame_header->src_mac[i] = src.bytes[i];
	}

	//set ethertype
	new_frame_header->ethertype = htons(ethertype);
	
	//copy in payload data
	for(i = 0; i < data_len; i++){
		new_frame_data[i] = data[i];
	}

	//set the return pointer
	*frame_return = new_frame;
	
out:
	return new_frame_len;
}
