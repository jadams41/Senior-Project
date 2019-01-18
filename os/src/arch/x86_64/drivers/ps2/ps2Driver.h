#include <stdint-gcc.h>

#ifndef PS2_DRIVER
#define PS2_DRIVER

char getScancode(void);
void ps2_init(void);
void initPs2();
void keyboard_config(void);
char ps2_poll_read(void);
char getchar(void);
void initialize_scancodes(void);
void initialize_shift_down_dict(void);
void block_until_response_available(void);
void block_until_input_ready(void);
void pollInputBuffer();
int8_t pollOutputBuffer();
void keyboard_handler_main(char scan);

#endif