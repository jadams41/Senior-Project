#include <stdint-gcc.h>

#include "drivers/cmos/cmos.h"
#include "types/timing.h"
#include "utils/printk.h"
#include "utils/utils.h"

static rtc_interrupt_state rtc_state = {
	.bcd_or_binary = 0,
	.hour_format = 0,
	.init_time = {
		.t = {
			.seconds = 0,
			.minutes = 0,
			.hours = 0
		},
		.d = {
			.day = 0,
			.weekday = 0,
			.month = 0,
			.year = 0
		},
	},
	.cur_time = {
		.t = {
			.seconds = 0,
			.minutes = 0,
			.hours = 0
		},
		.d = {
			.day = 0,
			.weekday = 0,
			.month = 0,
			.year = 0
		},
	},
	.rate = 0,
	.interrupts_per_sec = 0,
	.ints_received = 0,
	.enabled = 0,
	.read_cur_rtc_retries = 0,
};

char *weekdays[8] = {"Bad Weekday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static uint8_t CMOS_read_byte(uint8_t offset){
	uint8_t read_byte;

	if(offset > 0x5D){
		printk_err("invalid offset (max is 0x5D)\n");
		return 0;
	}
	
	//indicate that we want to read byte at supplied offset
	asm("CLI");
	outb(0x70, 0x80 | offset);

	read_byte = inb(0x71);
	asm("STI");

	return read_byte;
}

static void __attribute__((unused)) CMOS_write_byte(uint8_t offset, uint8_t value){
	if(offset > 0x5D){
		printk_err("invalid offset (max is 0x5D)\n");
		return;
	}

	//indicate that we want to write byte at supplied offset
	asm("CLI");
	outb(0x70, 0x80 | offset);

	/* int i; */
	/* while(i++ < 1000) ; */

	outb(0x71, value);
	asm("STI");
}


static void get_rtc_datetime_format(){
	uint8_t statusB = CMOS_read_byte(0xB);

	/* default values */
	rtc_state.hour_format = HOUR_FORMAT_12;
	rtc_state.bcd_or_binary = BCD_MODE;

	/* check if different format flags are set */
	if(statusB & 0b10)
		rtc_state.hour_format = HOUR_FORMAT_24;

	if(statusB & 0b100)
		rtc_state.bcd_or_binary = BINARY_MODE;
}

static int CMOS_update_in_progress(){
	uint8_t reg_a = CMOS_read_byte(CMOS_STATUS_REG_A);
	return (reg_a & 0x80);
}

static int lazy_update_time(){
	if(rtc_state.ints_received % rtc_state.interrupts_per_sec == 0){
	        if(++rtc_state.cur_time.t.seconds > 60){
			rtc_state.cur_time.t.seconds = 0;
			if(++rtc_state.cur_time.t.minutes > 60){
				rtc_state.cur_time.t.minutes = 0;
				if(++rtc_state.cur_time.t.hours > 24){
					rtc_state.cur_time.t.hours = 0;
					//todo: add logic for updating the date as well
				}
			}
		}
		
		return 1;
	}
	
	return 0;
}

void rtc_timer_isr(int irq, int err){
	rtc_state.ints_received += 1;
	if(lazy_update_time()){
		printk("cur time: %02d:%02d:%02d\n", rtc_state.cur_time.t.hours, rtc_state.cur_time.t.minutes, rtc_state.cur_time.t.seconds);
	}
	
	/* acknowledge the interrupt to CMOS by reading status register C */
	outb(0x70, 0x80 | CMOS_STATUS_REG_C);
	inb(0x71);
}

datetime read_cur_rtc(){
	datetime cur_datetime;
	uint8_t seconds, minutes, hours, weekday, day_of_month, month, year;
	uint8_t last_seconds, last_minutes, last_hours, last_weekday, last_dom, last_month, last_year;

	if(!rtc_state.bcd_or_binary || !rtc_state.hour_format){
		get_rtc_datetime_format();
	}

	//read registers until you get the same values twice in a row to avoid getting bad data due to RTC updates
	while(CMOS_update_in_progress()) ;
	seconds = CMOS_read_byte(CMOS_RTC_SECONDS);
	minutes = CMOS_read_byte(CMOS_RTC_MINUTES);
	hours = CMOS_read_byte(CMOS_RTC_HOURS);
	weekday = CMOS_read_byte(CMOS_RTC_DAY_OF_WEEK);
	day_of_month = CMOS_read_byte(CMOS_RTC_DATE_DAY);
	month = CMOS_read_byte(CMOS_RTC_DATE_MONTH);
	year = CMOS_read_byte(CMOS_RTC_DATE_YEAR);

	int times_read = 0;
	do {
		last_seconds = seconds;
		last_minutes = minutes;
		last_hours = hours;
		last_weekday = weekday;
		last_dom = day_of_month;
		last_month = month;
		last_year = year;

		seconds = CMOS_read_byte(CMOS_RTC_SECONDS);
		minutes = CMOS_read_byte(CMOS_RTC_MINUTES);
		hours = CMOS_read_byte(CMOS_RTC_HOURS);
		weekday = CMOS_read_byte(CMOS_RTC_DAY_OF_WEEK);
		day_of_month = CMOS_read_byte(CMOS_RTC_DATE_DAY);
		month = CMOS_read_byte(CMOS_RTC_DATE_MONTH);
		year = CMOS_read_byte(CMOS_RTC_DATE_YEAR);
		times_read += 1;
	} while ((last_seconds != seconds) ||
		 (last_minutes != minutes) ||
		 (last_hours != hours) ||
		 (last_weekday != weekday) ||
		 (last_dom != day_of_month) ||
		 (last_month != month) ||
		 (last_year != year));

	if(times_read > 1){
		rtc_state.read_cur_rtc_retries += (times_read - 1);
	}
	
	if(rtc_state.bcd_or_binary == BCD_MODE){
		seconds = (seconds & 0x0F) + ((seconds / 16) * 10);
		minutes = (minutes & 0x0F) + ((minutes / 16) * 10);
		hours = ( (hours & 0x0F) + (((hours & 0x70) / 16) * 10) ) | (hours & 0x80);
		day_of_month = (day_of_month & 0x0F) + ((day_of_month / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);
	}

        cur_datetime.t.seconds = seconds;
	cur_datetime.t.minutes = minutes;
	cur_datetime.t.hours = hours;

	cur_datetime.d.day = day_of_month;
	cur_datetime.d.weekday = weekday;
	cur_datetime.d.month = month;
	cur_datetime.d.year = year;

	return cur_datetime;
}

//taken from https://wiki.osdev.org/RTC
void CMOS_initialize_rtc_periodic_interrupts(RTC_periodic_int_rate rate){
	uint8_t prev_regB, prev_regA;

	/* get rtc format information from cmos */
	get_rtc_datetime_format();

	rtc_state.rate = rate;
	rtc_state.interrupts_per_sec = (32768 >> (rate - 1)); //convert request rate to hz
	rtc_state.ints_received = 0;
	rtc_state.read_cur_rtc_retries = 0;
	
        /* Turn on IRQ 8 */
	prev_regB = CMOS_read_byte(CMOS_STATUS_REG_B);
	CMOS_write_byte(CMOS_STATUS_REG_B, prev_regB | 0x40);

	/* set the interrupt rate to the fastest */
	prev_regA = CMOS_read_byte(CMOS_STATUS_REG_A);
	CMOS_write_byte(CMOS_STATUS_REG_A, (prev_regA & 0xF0) | (0xFF & rate));

	/* get the current timestamp before we enable interrupts */
        rtc_state.cur_time = rtc_state.init_time = read_cur_rtc();
	
	rtc_state.enabled = 1;
	
	/* finally, read status register C to enable interrupts */
	CMOS_read_byte(CMOS_STATUS_REG_C);
}

void print_current_time_and_date_from_rtc(){
	datetime cur =  read_cur_rtc();

	char *weekday_str = weekdays[0];
	if(cur.d.weekday >= 1 && cur.d.weekday <= 7){
		weekday_str = weekdays[cur.d.weekday];
	}
	
	printk_info("current time: %d:%d:%d\n", cur.t.hours, cur.t.minutes, cur.t.seconds);
	printk_info("current date: %s, %d/%d/%d%d\n", weekday_str, cur.d.month, cur.d.day, 20, cur.d.year);
}
