#ifndef UTILS
#define UTILS

//port interaction functions
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);
//void outd(uint16_t port, uint64_t val);
uint8_t  inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
//uint64_t ind(uint16_t port);

//page table functions
int entry_present(uint64_t entry);
uint64_t* strip_present_bits(uint64_t* addr_with_bits);

#endif
