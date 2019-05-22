#ifndef BYTE_ORDER
#define BYTE_ORDER

#include <stdint-gcc.h>

#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 2

uint8_t get_endianness();

uint16_t le16_to_cpu(uint16_t le16);
uint32_t le32_to_cpu(uint32_t le32);
uint64_t le64_to_cpu(uint32_t le64);

uint16_t be16_to_cpu(uint16_t be16);
uint32_t be32_to_cpu(uint32_t be32);
uint64_t be64_to_cpu(uint32_t be64);

uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);
uint16_t ntohs(uint16_t netshort);
uint32_t ntohl(uint32_t netlong);
#endif
