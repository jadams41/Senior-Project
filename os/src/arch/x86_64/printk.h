#ifndef PRINTK
#define PRINTK

extern void halt_wrapper();
extern void VGA_clear();
extern void VGA_display_char(char);
extern void VGA_display_str(const char *);

void printk(const char *fmtStr, ...);
void vgaDispCharTest();
void printkTest();
void disableSerialPrinting();
void enableSerialPrinting();

#endif