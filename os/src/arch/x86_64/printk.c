#include <stdarg.h>
#include "printk.h"
#include "serial.h"
#include <stdint-gcc.h>

void printCharToVGAandSER(char c){
	VGA_display_char(c);
	SER_write(&c,1);
}

void printLineAcrossScreen(){
	int i;
	for(i = 0; i < 80; i++){
		printCharToVGAandSER('-');
	}
}

/**
 * recursively prints an integer through modulo
 * and division manipulation
 */
void printInteger(int i){
	if(i < 0){
		printCharToVGAandSER('-');
		i *= -1;
	}
	if(i / 10){
		printInteger(i / 10);
	}
	printCharToVGAandSER((char)(48 + i % 10));
}

/**
 * same as printInteger but accepts an unsigned integer
 */
void printUnsignedInteger(unsigned int i){
	if(i / 10){
		printUnsignedInteger(i / 10);
	}
	printCharToVGAandSER((char)(48 + i % 10));
}

void printUnsignedHex(unsigned int i, short isUppercase){
	if(i / 16){
		printUnsignedHex(i / 16, isUppercase);
	}
	char charToPrint;
	i %= 16;
	if(i > 9){
		if(isUppercase){
			charToPrint = (char)(i + 55);
		}
		else{
			charToPrint = (char)(i + 87);
		}
	}
	else {
		charToPrint = (char)(48 + i);
	}
	printCharToVGAandSER(charToPrint);
}

void printShort(short s){
	int i = 0 + s;
	printInteger(i);
}

void printUnsignedShort(unsigned short s){
	unsigned int i = 0 + s;
	printUnsignedInteger(i);
}

void printHexShort(unsigned short s){
	printUnsignedHex(s, 0);
}

void printLong(long l){
	if(l < 0){
		printCharToVGAandSER('-');
		l *= -1;
	}
	if(l / 10){
		printLong(l / 10);
	}
	printCharToVGAandSER((char)(48 + l % 10));
}

void printUnsignedLong(unsigned long l){
	if(l / 10){
		printUnsignedLong(l / 10);
	}
	printCharToVGAandSER((char)(48 + l % 10));
}

void printHexLong(unsigned long l){
	if(l / 16){
		printHexLong(l / 16);
	}
	char charToPrint;
	l %= 16;
	if(l > 9){
		charToPrint = (char)(l + 87);
	}
	else {
		charToPrint = (char)(48 + l);
	}
	printCharToVGAandSER(charToPrint);
}

void printQuad(long long l){
	if(l / 10){
		printQuad(l / 10);
	}
	printCharToVGAandSER((char)(48 + l % 10));
}

void printUnsignedQuad(unsigned long long l){
	if(l / 10){
		printUnsignedQuad(l / 10);
	}
	printCharToVGAandSER((char)(48 + l % 10));
}

void printHexQuad(unsigned long long l){
	if(l / 16){
		printHexQuad(l / 16);
	}
	char charToPrint;
	l %= 16;
	if(l > 9){
		charToPrint = (char)(l + 87);
	}
	else {
		charToPrint = (char)(48 + l);
	}
	printCharToVGAandSER(charToPrint);
}
/**
 * currently includes support for the following tokens
 * %d integer
 * %s string
 * %% '%'
 * %u unsigned integer
 * %x hex
 * %c character
 * %p pointer
 * %h[dux] short [d] integer, [u] unsigned, or [h] hex
 * %l[dux] long [d] integer, [u] unsigned, or [h] hex
 * %q[dux] quad [d] integer, [u] unsigned, or [h] hex
 *
 * Note: for every character written, that character will
 * also be printed to the Serial Port #1
 */
void printk(const char *fmtStr, ...) {
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
				case '%':
					printCharToVGAandSER('%');
					break;
				case 'u':
					printUnsignedInteger(va_arg(args, unsigned int));
					break;
				case 'x':
					printUnsignedHex(va_arg(args, unsigned int), 0);
					break;
				case 'X':
					printUnsignedHex(va_arg(args, unsigned int), 1);
					break;
				case 'c':
					printCharToVGAandSER((char)va_arg(args, int));
					break;
				case 'p':
					printUnsignedHex((unsigned int)va_arg(args, unsigned int), 0);
					break;
				case 'h':
					switch(*(fmtStr + 2)){
						case 'd':
							printShort((unsigned short)va_arg(args, int));
							break;
						case 'u':
							printUnsignedShort((unsigned short)va_arg(args, unsigned int));
							break;
						case 'x':
							printHexShort((unsigned short)va_arg(args, unsigned int));
							break;
					}
					fmtStr++;
					break;
				case 'l':
					switch(*(fmtStr + 2)){
						case 'd':
							printLong(va_arg(args, long));
							break;
						case 'u':
							printUnsignedLong(va_arg(args, unsigned long));
							break;
						case 'x':
							printHexLong(va_arg(args, unsigned long));
							break;
					}
					fmtStr++;
					break;
				case 'q':
					switch(*(fmtStr + 2)){
						case 'd':
							printQuad(va_arg(args, long long));
							break;
						case 'u':
							printUnsignedQuad(va_arg(args, unsigned long long));
							break;
						case 'x':
							printHexQuad(va_arg(args, unsigned long long));
							break;
					}
					fmtStr++;
				default:
					//character was not supported, will skip over
					break;
			}
			fmtStr += 2;
		}
		else {
			printCharToVGAandSER(*fmtStr);
			fmtStr++;
		}
	}
}



void vgaDispCharTest(){
  printCharToVGAandSER('t');
  printCharToVGAandSER('e');
  printCharToVGAandSER('s');
  printCharToVGAandSER('t');
  printCharToVGAandSER('\n');
}

void printkTest(){
  int testInt;
  long l = 2147483647;
  short s = 32767;

  printLineAcrossScreen();
  printk("printk int test: (%d) ?= 483\n", 483);
  printk("printk str test: '%s' ?= 'here is a Test String'\n", "here is a Test String");
  printk("printk int and string test: int - %d, string - '%s'\n", 123451, "prev should be 123451");
  printk("printk escape char test: %%\n");
  printk("printk uint test: %u %u\n", 1, -1);
  printk("negative int test: %d\n", -2023);
  printk("printk hex test: upper - %X, lower - %x\n", 4444, 4444);
  printk("printk pointer test: %p \n", &testInt);
  printk("printk long test: %ld \n", l);
  printk("printk unsigned long test: %lu\n", -1);
  printk("printk hex long test: %lx\n", -1);
  printk("printk short test: %hd\n", s);
  printk("printk ushort test: %hu\n", -1);
  printk("printk hex short test: %hx\n", -1);
  printk("printk quad test: %qd \n", l);
  printk("printk unsigned quad test: %qu\n", -1);
  printk("printk hex quad test: %qx\n", -1);
  printLineAcrossScreen();

}
