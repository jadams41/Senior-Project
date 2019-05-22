#include <stdint-gcc.h>
#include "byte_order.h"

uint8_t get_endianness(){
    int v = 1;
    char *y = (char*)&v;
    
    return *y ? LITTLE_ENDIAN : BIG_ENDIAN;
}

uint16_t le16_to_cpu(uint16_t le16){
    if(get_endianness() == LITTLE_ENDIAN){
	return le16;
    }

    uint8_t *le_byte_ptr = (uint8_t*)&le16;

    uint16_t to_return = le_byte_ptr[0];
    to_return <<= 8;
    to_return += le_byte_ptr[1];

    return to_return;
}

uint32_t le32_to_cpu(uint32_t le32){
    if(get_endianness() == LITTLE_ENDIAN){
	return le32;
    }

    uint8_t *le_byte_ptr = (uint8_t*)&le32;

    uint32_t to_return = le_byte_ptr[0];
    to_return <<= 8;
    to_return += le_byte_ptr[1];
    to_return <<= 8;
    to_return += le_byte_ptr[2];
    to_return <<= 8;
    to_return += le_byte_ptr[3];

    return to_return;
}

uint64_t le64_to_cpu(uint32_t le64){
    if(get_endianness() == LITTLE_ENDIAN){
	return le64;
    }

    uint8_t *le_byte_ptr = (uint8_t*)&le64;
    
    uint64_t to_return = le_byte_ptr[0];
    to_return <<= 8;
    to_return += le_byte_ptr[1];
    to_return <<= 8;
    to_return += le_byte_ptr[2];
    to_return <<= 8;
    to_return += le_byte_ptr[3];
    to_return <<= 8;
    to_return += le_byte_ptr[4];
    to_return <<= 8;
    to_return += le_byte_ptr[5];
    to_return <<= 8;
    to_return += le_byte_ptr[6];
    to_return <<= 8;
    to_return += le_byte_ptr[7];

    return to_return;
}


uint16_t be16_to_cpu(uint16_t be16){
    if(get_endianness() == BIG_ENDIAN){
	return be16;
    }

    uint8_t *be_byte_ptr = (uint8_t*)&be16;

    uint16_t to_return = be_byte_ptr[0];
    to_return <<= 8;
    to_return += be_byte_ptr[1];

    return to_return;
}

uint32_t be32_to_cpu(uint32_t be32){
    if(get_endianness() == BIG_ENDIAN){
	return be32;
    }
    
    uint8_t *be_byte_ptr = (uint8_t*)&be32;

    uint32_t to_return = be_byte_ptr[0];
    to_return <<= 8;
    to_return += be_byte_ptr[1];
    to_return <<= 8;
    to_return += be_byte_ptr[2];
    to_return <<= 8;
    to_return += be_byte_ptr[3];

    return to_return;
}

uint64_t be64_to_cpu(uint32_t be64){
    if(get_endianness() == BIG_ENDIAN){
	return be64;
    }

    uint8_t *be_byte_ptr = (uint8_t*)&be64;
    
    uint64_t to_return = be_byte_ptr[0];
    to_return <<= 8;
    to_return += be_byte_ptr[1];
    to_return <<= 8;
    to_return += be_byte_ptr[2];
    to_return <<= 8;
    to_return += be_byte_ptr[3];
    to_return <<= 8;
    to_return += be_byte_ptr[4];
    to_return <<= 8;
    to_return += be_byte_ptr[5];
    to_return <<= 8;
    to_return += be_byte_ptr[6];
    to_return <<= 8;
    to_return += be_byte_ptr[7];

    return to_return;
}

uint16_t htons(uint16_t hostshort){
    if(get_endianness() == BIG_ENDIAN){
	return hostshort;
    }

    uint8_t *le_byte_ptr = (uint8_t*)&hostshort;

    uint16_t to_return = le_byte_ptr[0];
    to_return <<= 8;
    to_return += le_byte_ptr[1];

    return to_return;
}

uint32_t htonl(uint32_t hostlong){
    if(get_endianness() == BIG_ENDIAN){
	return hostlong;
    }

    uint8_t *le_byte_ptr = (uint8_t*)&hostlong;

    uint32_t to_return = le_byte_ptr[0];
    to_return <<= 8;
    to_return += le_byte_ptr[1];
    to_return <<= 8;
    to_return += le_byte_ptr[2];
    to_return <<= 8;
    to_return += le_byte_ptr[3];

    return to_return;
}

uint16_t ntohs(uint16_t netshort){
    if(get_endianness() == BIG_ENDIAN){
	return netshort;
    }

    uint8_t *be_byte_ptr = (uint8_t*)&netshort;

    uint16_t to_return = be_byte_ptr[0];
    to_return <<= 8;
    to_return += be_byte_ptr[1];

    return to_return;    
}

uint32_t ntohl(uint32_t netlong){
    if(get_endianness() == BIG_ENDIAN){
	return netlong;
    }

    uint8_t *be_byte_ptr = (uint8_t*)&netlong;

    uint32_t to_return = be_byte_ptr[0];
    to_return <<= 8;
    to_return += be_byte_ptr[1];
    to_return <<= 8;
    to_return += be_byte_ptr[2];
    to_return <<= 8;
    to_return += be_byte_ptr[3];

    return to_return;    
}

