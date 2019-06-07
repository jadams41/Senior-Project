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

//RTC interrupt service routine
void rtc_isr(int irq, int err){
        printk("test\n");

	/* Read register C in order to make sure that we receive interrupts */
	outb(0x70, 0x0C);       //select register C
	inb(0x71);              // throw away the contents
}

//taken from https://wiki.osdev.org/RTC
void initialize_rtc_periodic_interrupts(){
	asm("CLI");		          // important that no interrupts happen (perform a CLI)

	outb(0x70, CMOS_RTC_REG_A);	  // select Status Register A, and disable NMI (by setting the 0x80 bit)
	outb(0x71, 0x20);	          // write to CMOS/RTC RAM

	/* Read register C in order to make sure that we receive interrupts */
	outb(0x70, CMOS_RTC_REG_C);       //select register C
	inb(0x71);                        // throw away the contents
	
	asm("STI");		          // (perform an STI) and reenable NMI if you wish
}

void enable_periodic_rtc_interrupts(){
        asm("CLI");			// disable interrupts

	outb(0x70, CMOS_RTC_REG_B);		// select register B, and disable NMI
	uint8_t prev = inb(0x71);	// read the current value of register B
	outb(0x70, CMOS_RTC_REG_B);		// set the index again (a read will reset the index to register D)
	outb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

	asm("STI");
}

/* void set_periodic_rtc_interrupt_rate(uint8_t rate){ */
/* 	rate &= 0x0F;			// rate must be above 2 and not over 15 */
/* 	disable_ints(); */
/* 	outportb(0x70, 0x8A);		// set index to register A, disable NMI */
/* 	char prev=inportb(0x71);	// get initial value of register A */
/* 	outportb(0x70, 0x8A);		// reset index to A */
/* 	outportb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits. */
/* 	enable_ints(); */
/* 	frequency =  32768 >> (rate-1); */
/* } */
