#include <stdarg.h>

extern void halt_wrapper();
extern void VGA_clear();
extern void VGA_display_char(char);
extern void VGA_display_str(const char *);

/**
 * recursively prints an integer through modulo 
 * and division manipulation
 */
void printInteger(int i){
	if(i / 10){
		printInteger(i / 10);
	}
	VGA_display_char((char)(48 + i % 10));
}


/**
 * currently includes support for the following tokens
 * %d integer
 * %s string
 *
 * NOTE: currently can't escape %
 */
void printk(const char *fmtStr, ...){
	va_list args;
	va_start(args, fmtStr);

	while(1){
		if(!*fmtStr) break;
		if(*fmtStr == '%'){
			switch(*(fmtStr + 1)){
				case 'd':
					printInteger(va_arg(args, int));
					break;
				case 's':
					VGA_display_str(va_arg(args, char*));
					break;
				default:
					//not supported, will skip over
					break;
			}
			fmtStr += 2;
		}
		else {
			VGA_display_char(*fmtStr);
			fmtStr++;
		}
	}
}

int kmain(){
  int enabled = 0;

  //infinite loop to allow us to attach gdb before the program finishes executing
  //while(!enabled) {
  //;//halt_wrapper();
  //}

  //if we make it here, we know that gdb was attached and used to break out of infinite loop

  //clear the vga console now that we are up and running
  VGA_clear();

  VGA_display_char('t');
  VGA_display_char('e');
  VGA_display_char('s');
  VGA_display_char('t');
  VGA_display_char('\n');

  // while(!enabled) ;

  VGA_display_str("string print test\n");

  printk("printk int test: (%d) ?= 483\n", 483);
  printk("printk str test: '%s' ?= 'here is a Test String'\n", "here is a Test String");
  printk("printk int and string test: int - %d, string - '%s'\n", 123451, "prev should be 123451");
}