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
void printk_info(const char *fmtStr, ...);
void vgaDispCharTest();
void printkTest();
void setColor(char);
void setBackgroundColor(char);
char getColor();

int VGA_col_count();
int VGA_row_count();
void VGA_display_attr_char(int, int, char, char, char);

#define VGA_BLACK 0x00
#define VGA_BLUE 0x01
#define VGA_GREEN 0x02
#define VGA_CYAN 0x03
#define VGA_RED 0x04
#define VGA_MAGENTA 0x05
#define VGA_BROWN 0x06
#define VGA_GRAY 0x07
#define VGA_DARK_GRAY 0x08
#define VGA_BRIGHT_BLUE 0x09
#define VGA_BRIGHT_GREEN 0x0a
#define VGA_BRIGHT_CYAN 0x0b
#define VGA_BRIGHT_RED 0x0c
#define VGA_BRIGHT_MAGENTA 0x0d
#define VGA_YELLOW 0x0e
#define VGA_WHITE 0x0f

#endif
