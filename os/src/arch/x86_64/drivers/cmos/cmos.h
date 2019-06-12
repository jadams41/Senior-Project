/* driver for interacting with the CMOS RAM Memory (including CMOS RTC) */
#ifndef _CMOS_H
#define _CMOS_H

#include <stdint-gcc.h>
#include "types/timing.h"

/* CMOS memory-mapped ports */
#define CMOS_ADDRESS_PORT      0x70
#define CMOS_DATA_PORT         0x71

/* CMOS memory layout (registers must be read using the CMOS ports) */
//RTC fields
#define CMOS_RTC_SECONDS       0x00
#define CMOS_RTC_SECONDS_ALARM 0x01
#define CMOS_RTC_MINUTES       0x02
#define CMOS_RTC_MINUTES_ALARM 0x03
#define CMOS_RTC_HOURS         0x04
#define CMOS_RTC_HOURS_ALARM   0x05
#define CMOS_RTC_DAY_OF_WEEK   0x06
#define CMOS_RTC_DATE_DAY      0x07
#define CMOS_RTC_DATE_MONTH    0x08
#define CMOS_RTC_DATE_YEAR     0x09

//Status registers

/* Status Register A:
   - bit 7    : update in progress (0 => date and time can be read, 1 => time update in progress
   - bits 6-4 : time frequency divider
   - bits 3-0 : rate selection frequency */
#define CMOS_STATUS_REG_A      0x0A

/* Status Register B:
   - bit 7 : clock update cycle (0 = update normally, 1 = Abort update in progress)
   - bit 6 : periodic interrupt (0 = disable interrupt (default), 1 = enable interrupt)
   - bit 5 : alarm interrupt (0 = disable interrupt (default), 1 = enable interrupt)
   - bit 4 : update ended interrupt (0 = disable interrupt (default), 1 = enable interrupt)
   - bit 3 : status register A square wave freq (0 = disable square wave frequency, 1 = enable square wave)
   - bit 2 : 24 hour clock (0 = 24 clock, 1 = 12 hour)
   - bit 1 : daylight savings time (0 = disable daylight savings, 1 = enable daylight savings */
#define CMOS_STATUS_REG_B      0x0B

#define CMOS_STATUS_REG_C      0x0C
#define CMOS_STATUS_REG_D      0x0D


/* Datetime data structure (status register B indicates binary or bcd and 12 or 24 hour format) */
//bit 2 (0b100) in Status register B (if set, binary mode, otherwise BCD mode)
#define BINARY_MODE 1    //normal.               value 24 represented as 0x18 = 0b00011000
#define BCD_MODE    2    //Binary Coded Decimal. value 24 represented as 0x24 = 0b00100100
//bit 1 (0b10) in Status register B (if set, 24 hour format, otherwise 12 hour format)
#define HOUR_FORMAT_12 1 
#define HOUR_FORMAT_24 2

/* RTC interrupt rates
 frequency = 32768 >> (rate-1);
 rate = 3  -> frequency = 8192 Hz (TOO FAST, BLOWS SYSTEM UP)
 rate = 4  -> frequency = 4096 Hz (TOO FAST, BLOWS SYSTEM UP)
 rate = 5  -> frequency = 2048 Hz (TOO FAST, BLOWS SYSTEM UP)
 rate = 6  -> frequency = 1024 Hz
 rate = 7  -> frequency =  512 Hz
 rate = 8  -> frequency =  256 Hz
 rate = 9  -> frequency =  128 Hz
 rate = 10 -> frequency =   64 Hz
 rate = 11 -> frequency =   32 Hz
 rate = 12 -> frequency =   16 Hz
 rate = 13 -> frequency =    8 Hz
 rate = 14 -> frequency =    4 Hz
 rate = 15 -> frequency =    2 Hz
*/

typedef enum RTC_periodic_int_rate RTC_periodic_int_rate;
enum RTC_periodic_int_rate {
        RTC_ClockRate8kHz  = 3,
	RTC_ClockRate4kHz  = 4,
	RTC_ClockRate2kHz  = 5,
	RTC_ClockRate1kHz  = 6,
	RTC_ClockRate512Hz = 7,
	RTC_ClockRate256Hz = 8,
	RTC_ClockRate128Hz = 9,
	RTC_ClockRate64Hz  = 10,
	RTC_ClockRate32Hz  = 11,
	RTC_ClockRate16Hz  = 12,
	RTC_ClockRate8Hz   = 13,
	RTC_ClockRate4Hz   = 14,
	RTC_ClockRate2Hz   = 15,
};

#define RTC_RATE_FASTEST RTC_ClockRate8kHz
#define RTC_RATE_SLOWEST RTC_ClockRate2Hz

typedef struct rtc_interrupt_state rtc_interrupt_state; 
struct rtc_interrupt_state {
	/* rtc configuration information */
	int bcd_or_binary;
	int hour_format;
	
	/* info for keeping track of current time with interrupts */
	datetime init_time;
	datetime cur_time;
	
	uint8_t rate;
	uint32_t interrupts_per_sec;
	uint64_t ints_received;

	int enabled;

	/* statistics */
	int read_cur_rtc_retries;
	
};


void rtc_timer_isr(int irq, int err);
void CMOS_initialize_rtc_periodic_interrupts(RTC_periodic_int_rate rate);
void print_current_time_and_date_from_rtc();

#endif
