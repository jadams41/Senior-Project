#ifndef UTILS
#define UTILS

//port interaction functions
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

//page table functions
int entry_present(uint64_t entry);
uint64_t* strip_present_bits(uint64_t* addr_with_bits);

//string functions
void strcpy(char *src, char *dest);
void strncpy(char *src,  char *dest, int n);

#endif
