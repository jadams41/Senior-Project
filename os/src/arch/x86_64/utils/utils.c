#include <stdint-gcc.h>

// write out byte to port
void inline outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// read in byte from port
uint8_t inline inb(uint16_t port) {
    uint8_t val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

// write out word to port (2 bytes)
void inline outw(uint16_t port, uint16_t val){
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

uint16_t inline inw(uint16_t port){
    uint16_t val;
    asm volatile ("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void inline outl(uint16_t port, uint32_t val){
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

uint32_t inline inl(uint16_t port){
    uint32_t val;
    asm volatile ("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* void inline outd(uint16_t port, uint64_t val){ */
/*     asm volatile("outd %0, %1" : : "a"(val), "Nd"(port)); */
/* } */

/* uint64_t inline ind(uint16_t port){ */
/*     uint64_t val; */
/*     asm volatile ("ind %1, %0" : "=a"(val) : "Nd"(port)); */
/*     return val; */
/* } */

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
