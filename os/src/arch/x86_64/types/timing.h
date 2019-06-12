#ifndef _TIMING_H
#define _TIMING_H

#include <stdint-gcc.h>

typedef struct time time;
struct time {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
};

typedef struct date date;
struct date {
	uint8_t day;
	uint8_t weekday; //1 for Sunday, 7 for Saturday
	uint8_t month;
	uint8_t year;
};

typedef struct datetime datetime;
struct datetime {
	time t;
	date d;
};

/* RDTSC Instruction:
   Read Time-Stamp Counter

   returns the number of clock cycles since system loaded

   high order bits loaded into the EDX register (32 bits)
   low order bits loaded into the EAX register (32 bits)
*/

uint64_t get_num_clock_cycles();

#define RTC_IO_PORT1 0x70 // used to specify index or "register number" and to disable NMI
#define RTC_IO_PORT2 0x71 // used to read or write from/to that byte of CMOS config space

#define CMOS_RTC_REG_A 0x0A
#define CMOS_RTC_REG_B 0x0B
#define CMOS_RTC_REG_C 0x0C

void initialize_rtc_periodic_interrupts();
void enable_periodic_rtc_interrupts();

void print_current_time_and_date_from_rtc();
#endif
