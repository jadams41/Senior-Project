#ifndef PS2_DRIVER
#define PS2_DRIVER

char getScancode(void);
void ps2_init(void);
void keyboard_config(void);
char ps2_poll_read(void);
char getchar();
void initialize_scancodes();
void initialize_shift_down_dict();

#endif