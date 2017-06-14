#include <stdint-gcc.h>


void inline outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inline inb(uint16_t port) {
    uint8_t val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void inline outw(uint16_t port, uint16_t value){
    asm volatile("outw %%ax, %%dx": :"d" (port), "a" (value));
}

uint16_t inline inw(uint16_t port){
    uint16_t ret;
	asm volatile("inw %%dx, %%ax":"=a"(ret):"d"(port));
	return ret;
}

/***** page table functions *****/
int entry_present(uint64_t entry){
    //NOTE: this only checks the present bits and not the on demand bits
    //NOTE: for this reason it should only be used to check p4,p3, and p2 entries NOT p1!
    return entry != 0 && (entry & 0b11) != 0;
}

uint64_t* strip_present_bits(uint64_t* addr_with_bits){
    uint64_t present_bits = 0b111111111111;
    uint64_t present_inv = ~present_bits;

    uint64_t addr_num = (uint64_t)addr_with_bits;
    addr_num &= present_inv;

    return (uint64_t*)addr_num;
}

/***** string functions *****/
void strcpy(char *src, char *dest){
    while(*src){
        *dest = *src;
        dest++;
        src++;
    }
}

void strncpy(char *src,  char *dest, int n){
    while(n-- > 0){
        *dest = *src;
        dest++;
        src++;
    }
}
