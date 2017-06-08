#ifndef KEYBOARD_DRIVER
#define KEYBOARD_DRIVER

#include "process.h"

typedef struct {
    char kbd_buffer[160];
    char *read, *write, *buf_end;
    ProcessQueue blocked;
    int buffLen;
} KBD_state;

void init_kbd_state();

char KBD_read();
void KBD_write(char);
#endif
