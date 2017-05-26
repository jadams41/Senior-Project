#ifndef PRINTK
#define PRINTK

#include <stdint-gcc.h>

extern void halt_wrapper();
extern void VGA_clear();
extern void VGA_display_char(char);
extern void VGA_display_str(const char *);
extern uint64_t cur_char_color;

void printk(const char *fmtStr, ...);
void printk_err(const char *fmtStr, ...);
void printk_warn(const char *fmtStr, ...);
void vgaDispCharTest();
void printkTest();
void setColor(char);
void setBackgroundColor(char);
char getColor();

#define BLACK 0x00
#define BLUE 0x01
#define GREEN 0x02
#define CYAN 0x03
#define RED 0x04
#define MAGENTA 0x05
#define BROWN 0x06
#define GRAY 0x07
#define DARK_GRAY 0x08
#define BRIGHT_BLUE 0x09
#define BRIGHT_GREEN 0x0a
#define BRIGHT_CYAN 0x0b
#define BRIGHT_RED 0x0c
#define BRIGHT_MAGENTA 0x0d
#define YELLOW 0x0e
#define WHITE 0x0f

#endif
