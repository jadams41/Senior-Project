#include <stdint-gcc.h>
#include "utils/printk.h"
#include "types/timing.h"
#include "utils/utils.h"
#include "drivers/interrupts/idt.h"

static __inline__ uint64_t __rdtsc(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

/* Reads the RDTSC register using gcc's builtin __rdstc function */
uint64_t get_num_clock_cycles(){
	return __rdtsc();
}
