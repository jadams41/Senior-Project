#include <stdint-gcc.h>

#include "serial.h"
#include "utils.h"
#include "printk.h"

State curState;
int stateInitialized = 0;
static int hwBusy = 1;

static void init_state(State *state){
	state->head = &state->buff[0];
	state->tail = &state->buff[0];
}

/*
 * returns 0 if fifo not ready, 1 if ready
 */
static int serial_transmit_fifo_ready(){
	return inb(COM_PORT_1 + LINE_STATUS) & 0x20;
}

/*
 * write to the hardware
 */
static void consume_byte(char b){
	//even though this should be called from an interrupt when the hw is ready
	//still make sure that the FIFO is clear
	int status = serial_transmit_fifo_ready();
	while(!status)
		status = serial_transmit_fifo_ready();

	//fifo is empty and good to be written to
	outb(COM_PORT_1 + SER_DATA, (uint8_t) b);
}

static void consumer_next(State *state){
	if(state->head == state->tail){
		return;
	}
	char *c_to_consume = state->head++;
	consume_byte(*c_to_consume);
	if(state->head >= &state->buff[BUFF_SIZE]){
		state->head = &state->buff[0];
	}
}

static int producer_add_char(char toAdd, State *state){
	if(state->head - 1 == state->tail || (state->head == &state->buff[0] && state->tail == &state->buff[BUFF_SIZE - 1]))
		return 0;

	*state->tail++ = toAdd;
	if(state->tail >= &state->buff[BUFF_SIZE])
		state->tail = &state->buff[0];
	return 1;
}

void SER_init(){
	if(!stateInitialized){
		init_state(&curState);
		stateInitialized = 1;
	}

	//maybe need to set the baud rate
	//going to ignore this for now, will reevaluate if shit doesn't work

	//set the number of data bits to 8
	outb(COM_PORT_1 + LINE_CTRL, 0x03);

	//configure interrupts
	uint8_t configByte;
	InterruptRegister *conf = (InterruptRegister*)&configByte;
	conf->data_available_interrupt = 0;
	conf->transmitter_empty_interrupt = 1;
	conf->break_error_interrupt = 0;
	conf->status_change_interrupt = 1;
	conf->zeros = 0;

	outb(COM_PORT_1 + INTERRUPT_ENABLE, (uint8_t)configByte);
}

static void clearLineStatus(){
	//grab the line status
	/*uint8_t line_status= */inb(COM_PORT_1 + LINE_STATUS);
	/*uint8_t iir = */inb(COM_PORT_1 + INTERRUPT_ID);

}

/*
 * @param buff pointer to the beginning of a string to print
 * @param len the length of buff
 * @return the number of characters written to the buffer
 *
 * NOTE: this function performs 2 critical tasks
 * 1) pushes the characters in buff onto the circualar driver Serial buffer
 * 2) checks if hw is busy and if not, then consumes a byte
 *
 * since consume byte was not mentioned in the interface,
 * this function can act solely as a wrapper for it,
 * in order to do this (and solely use task #2), pass in an empty buffer and len=0
 */

int SER_write(const char *buff, int len){

	int buffFull = 0, charsWritten = 0;
	while(len--){
		buffFull = !producer_add_char(*buff, &curState);
		buff += sizeof(char);
		if(buffFull){
			disableSerialPrinting();
			// printk("failed writing to serial driver buffer, ran out of buffer space\n");
			enableSerialPrinting();
			return charsWritten;
		}
		charsWritten++;
	}

	hwBusy = !serial_transmit_fifo_ready();

	//hardware is ready => write byte to fifo
	if(!hwBusy){
		consumer_next(&curState);
	}
	//line status indicates hardware is busy
	//clear the overrun error bit and do not attempt to write
	else {
		clearLineStatus();
	}

	return charsWritten;
}
