#ifndef SERIAL
#define SERIAL

#include <stdint-gcc.h>

#define BUFF_SIZE 160
#define COM_PORT_1 0x3F8
#define COM_PORT_2 0x2F8

//Serial port offsets
#define SER_DATA 0
#define INTERRUPT_ENABLE 1
#define INTERRUPT_ID 2
#define LINE_CTRL 3
#define MODEM_CTRL 4
#define LINE_STATUS 5
#define MODEM_STATUS 6
#define SCRATCH 7


typedef struct {
	char buff[BUFF_SIZE];
	char *head, *tail;
} State;

typedef struct {
	uint8_t data_available_interrupt:1;
	uint8_t transmitter_empty_interrupt:1;
	uint8_t break_error_interrupt:1;
	uint8_t status_change_interrupt:1;
	uint8_t zeros:4;

}__attribute__((packed)) InterruptRegister;

void SER_init();
int SER_write(const char *buff, int len);
void disableSerialPrinting();
void enableSerialPrinting();

#endif
