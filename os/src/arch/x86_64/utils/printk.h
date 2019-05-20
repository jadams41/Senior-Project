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
void printk_dir(const char *fmtStr, ...);
void printk_rainbow(const char *fmtStr, ...);

void vgaDispCharTest();
void printkTest();

void setColor(char);
void setBackgroundColor(char);
char getColor();
char getBackgroundColor();

/* functions required to run the snakes test program */
int VGA_col_count();
int VGA_row_count();
void VGA_display_attr_char(int, int, char, char, char);

/*** VGA Color Codes ***/
//same codes for both foreground and background color
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

/*** Serial Color Codes ***/

/* reset codes */
#define SER_COLOR_RESET "\033[39m" //resets foreground color to terminal default
#define SER_BACK_RESET  "\033[49m" //resets background color to terminal default
#define SER_COLOR_OFF   "\033[0m"  //resets both foreground and background colors

/* Serial Foreground Color Codes */
#define SER_BLACK          "\033[0;30m"
#define SER_BLUE           "\033[0;34m"
#define SER_GREEN          "\033[0;32m"
#define SER_CYAN           "\033[0;36m"
#define SER_RED            "\033[0;31m"
#define SER_MAGENTA        "\033[0;35m"    //purple
#define SER_BROWN          "\033[0;33m"    //yellow (described as orange/brown)
#define SER_GRAY           "\033[0;37m"    //white (described as light gray)
#define SER_DARK_GRAY      "\033[1;30m"
#define SER_BRIGHT_BLUE    "\033[0;94m"
#define SER_BRIGHT_GREEN   "\033[0;92m"
#define SER_BRIGHT_CYAN    "\033[0;96m"
#define SER_BRIGHT_RED     "\033[0;91m"
#define SER_BRIGHT_MAGENTA "\033[0;95m"
#define SER_YELLOW         "\033[0;93m"    //high intensity yellow
#define SER_WHITE          SER_COLOR_RESET //using reset text color for this (since white is default vga color and terminal default may not be white)

/* Serial Background Color Codes */
#define SER_BACK_BLACK          SER_BACK_RESET //using reset background color for this (since black is default vga color and terminal default may not be black)
#define SER_BACK_BLUE           "\033[44m"
#define SER_BACK_GREEN          "\033[42m"
#define SER_BACK_CYAN           "\033[46m"
#define SER_BACK_RED            "\033[41m"
#define SER_BACK_MAGENTA        "\033[45m"     //purple
#define SER_BACK_BROWN          "\033[43m"     //yellow (described as orange/brown)
#define SER_BACK_GRAY           "\033[47m"     //white (described as light gray)
#define SER_BACK_DARK_GRAY      "\033[0;100m"  //high intensity black
#define SER_BACK_BRIGHT_BLUE    "\033[0;104m"
#define SER_BACK_BRIGHT_GREEN   "\033[0;102m"
#define SER_BACK_BRIGHT_CYAN    "\033[0;106m"
#define SER_BACK_BRIGHT_RED     "\033[0;101m"
#define SER_BACK_BRIGHT_MAGENTA "\033[0;105m"
#define SER_BACK_YELLOW         "\033[0;103m"  //high intensity yellow
#define SER_BACK_WHITE          "\033[0;97m"   //high intensity white

#endif
