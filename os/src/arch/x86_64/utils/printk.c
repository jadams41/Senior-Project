#include <stdarg.h>
#include <stdint-gcc.h>
#include "printk.h"
#include "drivers/serial/serial.h"
#include "types/string.h"

extern uint64_t vga_buf_cur;
extern uint64_t vga_scroll_disabled;

//globals
static int printWidth = 0;
static int spacesOrZeros = 0; //spaces=0 (default) zeros=1
static int print_rainbow_mode = 0;

/* note: these are static because VGA color and serial color should not become disjunct
 *       calling setColor and setBackground color will call these methods for you */
static void setSerialColor(char newVgaColor);
static void setSerialBackgroundColor(char newVgaColor);


static int setPrintWidth(char *fmt){
    int newPrintWidth = 0;
    int stillParsing = 1;
    int numCharsConsumed = 0;

    spacesOrZeros = (*fmt == '0');

    //todo don't think this properly handles print width >= 10
    while(stillParsing){
	newPrintWidth *= 10;
	numCharsConsumed += 1;
	switch(*fmt){
	case '0':
	    break;
	case '1':
	    newPrintWidth += 1;
	    break;
	case '2':
	    newPrintWidth += 2;
	    break;
	case '3':
	    newPrintWidth += 3;
	    break;
	case '4':
	    newPrintWidth += 4;
	    break;
	case '5':
	    newPrintWidth += 5;
	    break;
	case '6':
	    newPrintWidth += 6;
	    break;
	case '7':
	    newPrintWidth += 7;
	    break;
	case '8':
	    newPrintWidth += 8;
	    break;
	case '9':
	    newPrintWidth += 9;
	    break;
	default:
	    stillParsing = 0;
	    numCharsConsumed -= 1;
	    newPrintWidth /= 10;
	}
	fmt++;
    }
    printWidth = newPrintWidth;
    return numCharsConsumed;
}

static void setSerialColor(char newVgaColor){
    switch(newVgaColor){
    case VGA_BLACK:
	SER_write(SER_BLACK, strlen(SER_BLACK));
	break;
    case VGA_BLUE:
	SER_write(SER_BLUE, strlen(SER_BLUE));
	break;
    case VGA_GREEN:
	SER_write(SER_GREEN, strlen(SER_GREEN));
	break;
    case VGA_CYAN:
	SER_write(SER_CYAN, strlen(SER_CYAN));
	break;
    case VGA_RED:
	SER_write(SER_RED, strlen(SER_RED));
	break;
    case VGA_MAGENTA:
	SER_write(SER_MAGENTA, strlen(SER_MAGENTA));
	break;
    case VGA_BROWN:
	SER_write(SER_BROWN, strlen(SER_BROWN));
	break;
    case VGA_GRAY:
	SER_write(SER_GRAY, strlen(SER_GRAY));
	break;
    case VGA_DARK_GRAY:
	SER_write(SER_DARK_GRAY, strlen(SER_DARK_GRAY));
	break;
    case VGA_BRIGHT_BLUE:
	SER_write(SER_BRIGHT_BLUE, strlen(SER_BRIGHT_BLUE));
	break;
    case VGA_BRIGHT_GREEN:
	SER_write(SER_BRIGHT_GREEN, strlen(SER_BRIGHT_GREEN));
	break;
    case VGA_BRIGHT_CYAN:
	SER_write(SER_BRIGHT_CYAN, strlen(SER_BRIGHT_CYAN));
	break;
    case VGA_BRIGHT_RED:
	SER_write(SER_BRIGHT_RED, strlen(SER_BRIGHT_RED));
	break;
    case VGA_BRIGHT_MAGENTA:
	SER_write(SER_MAGENTA, strlen(SER_MAGENTA));
	break;
    case VGA_YELLOW:
	SER_write(SER_YELLOW, strlen(SER_YELLOW));
	break;
    case VGA_WHITE:
	SER_write(SER_WHITE, strlen(SER_WHITE));
	break;
    default:
	printk("Unrecognized VGA color, halting so can figure out what's going on\n");
	asm("hlt");
    }
}

static void setSerialBackgroundColor(char newVgaColor){
    switch(newVgaColor){
    case VGA_BLACK:
	SER_write(SER_BACK_BLACK, strlen(SER_BACK_BLACK));
	break;
    case VGA_BLUE:
	SER_write(SER_BACK_BLUE, strlen(SER_BACK_BLUE));
	break;
    case VGA_GREEN:
	SER_write(SER_BACK_GREEN, strlen(SER_BACK_GREEN));
	break;
    case VGA_CYAN:
	SER_write(SER_BACK_CYAN, strlen(SER_BACK_CYAN));
	break;
    case VGA_RED:
	SER_write(SER_BACK_RED, strlen(SER_BACK_RED));
	break;
    case VGA_MAGENTA:
	SER_write(SER_BACK_MAGENTA, strlen(SER_BACK_MAGENTA));
	break;
    case VGA_BROWN:
	SER_write(SER_BACK_BROWN, strlen(SER_BACK_BROWN));
	break;
    case VGA_GRAY:
	SER_write(SER_BACK_GRAY, strlen(SER_BACK_GRAY));
	break;
    case VGA_DARK_GRAY:
	SER_write(SER_BACK_DARK_GRAY, strlen(SER_BACK_DARK_GRAY));
	break;
    case VGA_BRIGHT_BLUE:
	SER_write(SER_BACK_BRIGHT_BLUE, strlen(SER_BACK_BRIGHT_BLUE));
	break;
    case VGA_BRIGHT_GREEN:
	SER_write(SER_BACK_BRIGHT_GREEN, strlen(SER_BACK_BRIGHT_GREEN));
	break;
    case VGA_BRIGHT_CYAN:
	SER_write(SER_BACK_BRIGHT_CYAN, strlen(SER_BACK_BRIGHT_CYAN));
	break;
    case VGA_BRIGHT_RED:
	SER_write(SER_BACK_BRIGHT_RED, strlen(SER_BACK_BRIGHT_RED));
	break;
    case VGA_BRIGHT_MAGENTA:
	SER_write(SER_BACK_MAGENTA, strlen(SER_BACK_MAGENTA));
	break;
    case VGA_YELLOW:
	SER_write(SER_BACK_YELLOW, strlen(SER_BACK_YELLOW));
	break;
    case VGA_WHITE:
	SER_write(SER_BACK_WHITE, strlen(SER_BACK_WHITE));
	break;
    default:
	printk("Unrecognized VGA color, halting so can figure out what's going on\n");
	asm("hlt");
    }
}

void setColor(char newColor){
    cur_char_color &= 0xF0FF;
    cur_char_color |= (newColor << 8);

    setSerialColor(newColor);
}

void setBackgroundColor(char newColor){
    cur_char_color &= 0x0FFF;
    cur_char_color |= (newColor << 12);

    setSerialBackgroundColor(newColor);
}

char getColor(){
    return (cur_char_color & 0xF00) >> 8;
}

char getBackgroundColor(){
    return (cur_char_color & 0xF000) >> 12;
}

void printCharToVGAandSER(char c){
    static const char rainbow[] = {VGA_BRIGHT_RED,
			      VGA_BROWN,
			      VGA_YELLOW,
			      VGA_BRIGHT_GREEN,
			      VGA_BLUE,
			      VGA_BRIGHT_BLUE,
			      VGA_BRIGHT_MAGENTA};
    static const uint8_t rainbow_len = 7;

    if(print_rainbow_mode){
	setColor(rainbow[print_rainbow_mode++ - 1]);
	if(print_rainbow_mode == (rainbow_len + 1)){
	    print_rainbow_mode = 1;
	}
    }
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
void printInteger(int i, int recur){
    int numPrinted = 1; //last digit
    int printMinus = 0;
    if(i < 0){
	if(!spacesOrZeros){
	    printMinus = 1;
	}
	else {
	    printCharToVGAandSER('-');
	}
	i *= -1;
	numPrinted += 1;
    }
    if(!recur){
	int j = 10;
	while(1){
	    if(!(i / j)) break;
	    numPrinted += 1;
	    j *= 10;
	}
	while(numPrinted++ < printWidth){
	    printCharToVGAandSER(spacesOrZeros ? '0' : ' ');
	}
    }
    if(printMinus) printCharToVGAandSER('-');
    if(i / 10){
	printInteger(i / 10, 1);
    }
    printCharToVGAandSER((char)(48 + i % 10));
}

/**
 * same as printInteger but accepts an unsigned integer
 */
void printUnsignedInteger(unsigned int i, int recur){
    int numPrinted = 1;
    if(!recur){
	unsigned long j = 10;
	while(1){
	    if(!(i / j)) break;
	    numPrinted += 1;
	    j *= 10;
	}
	while(numPrinted++ < printWidth){
	    printCharToVGAandSER(spacesOrZeros ? '0' : ' ');
	}
    }
    if(i / 10){
	printUnsignedInteger(i / 10, 1);
    }
    printCharToVGAandSER((char)(48 + i % 10));
}

void printUnsignedHex(unsigned int i, short isUppercase, int recur){
    int numPrinted = 1;
    if(!recur){
        unsigned long j = 16;
	while(1){
	    if(!(i / j)) break;
	    numPrinted += 1;
	    j *= 16;
	}
	while(numPrinted++ < printWidth){
	    printCharToVGAandSER(spacesOrZeros ? '0' : ' ');
	}
    }
    if(i / 16){
	printUnsignedHex(i / 16, isUppercase, 1);
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
    printInteger(i, 0);
}

void printUnsignedShort(unsigned short s){
    unsigned int i = 0 + s;
    printUnsignedInteger(i, 0);
}

void printHexShort(unsigned short s){
    printUnsignedHex(s, 0, 0);
}

void printShortShort(char s){
    int i = 0 + s;
    printInteger(i, 0);
}

void printUnsignedShortShort(unsigned char s){
    unsigned int i = 0 + s;
    printUnsignedInteger(i, 0);
}

void printHexShortShort(unsigned char s){
    printUnsignedHex(s, 0, 0);
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

static void printk_var(const char *fmtStr, va_list args){
    char *str;

    while(1){
	if(!*fmtStr) break;
	if(*fmtStr == '%'){
	    fmtStr += setPrintWidth((char*)fmtStr + 1);
	    switch(*(fmtStr + 1)){
	    case 'd':
		printInteger(va_arg(args, int), 0);
		break;
	    case 's':
		str = va_arg(args, char*);
		VGA_display_str(str);
		while(*str){
		    SER_write(str++, 1);
		}
		break;
	    case '%':
		printCharToVGAandSER('%');
		break;
	    case 'u':
		printUnsignedInteger(va_arg(args, unsigned int), 0);
		break;
	    case 'x':
		printUnsignedHex(va_arg(args, unsigned int), 0, 0);
		break;
	    case 'X':
		printUnsignedHex(va_arg(args, unsigned int), 1, 0);
		break;
	    case 'c':
		printCharToVGAandSER((char)va_arg(args, int));
		break;
	    case 'p':
		printUnsignedHex((unsigned int)va_arg(args, unsigned int), 0, 0);
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
		    
		case 'h':
		    switch(*(fmtStr + 3)){
		    case 'd':
			printShortShort((unsigned short)va_arg(args, int));
			break;
		    case 'u':
			printUnsignedShortShort((unsigned short)va_arg(args, unsigned int));
			break;
		    case 'x':
			printHexShortShort((unsigned short)va_arg(args, unsigned int));
			break;
		    }

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
    printk_var(fmtStr, args);
}

void printk_err(const char *fmtStr, ...){
    va_list args;
    va_start(args, fmtStr);
    char color = getColor(), back = getBackgroundColor();
    setColor(VGA_BRIGHT_RED);
    printk("[ERR]: ");
    printk_var(fmtStr, args);
    setColor(color);
    setBackgroundColor(back);
}

void printk_warn(const char *fmtStr, ...){
    va_list args;
    va_start(args, fmtStr);
    char color = getColor(), back = getBackgroundColor();
    setColor(VGA_CYAN);
    printk("[WARN]: ");
    printk_var(fmtStr, args);
    setColor(color);
    setBackgroundColor(back);
}

void printk_info(const char *fmtStr, ...){
    va_list args;
    va_start(args, fmtStr);
    char color = getColor(), back = getBackgroundColor();
    setColor(VGA_BRIGHT_GREEN);
    printk("[INFO]: ");
    printk_var(fmtStr, args);
    setColor(color);
    setBackgroundColor(back);
}

/* What's the point of building an operating system if you can't have a little bit of fun? */
void printk_rainbow(const char *fmtStr, ...){
    va_list args;
    va_start(args, fmtStr);

    char color = getColor(), back = getBackgroundColor();
    print_rainbow_mode = 1;
    
    printk_var(fmtStr, args);

    setColor(color);
    setBackgroundColor(back);
    print_rainbow_mode = 0;
}

void printk_dir(const char *fmtStr, ...){
    va_list args;
    va_start(args, fmtStr);
    char color = getColor(), back = getBackgroundColor();
    setColor(VGA_BRIGHT_GREEN);
    printk_var(fmtStr, args);
    setColor(color);
    setBackgroundColor(back);
}

int VGA_col_count(){
    return 80;
}

int VGA_row_count(){
    return 25;
}

// void VGA_clear(){
// 	int i = 80;
// 	while(i--)
// 		VGA_display_char(0x08);
// }

void VGA_display_attr_char(int x, int y, char c, char fg, char bg){
    vga_scroll_disabled = 1;

    //store current fg and bg
    char color = getColor();
    char bcolor = getBackgroundColor();

    //set fg and bg
    setColor(fg);
    setBackgroundColor(bg);

    //store current position
    uint64_t current_pos = vga_buf_cur;

    //change the position
    vga_buf_cur = (y * VGA_col_count() + x) * 2 + 0xb8000;

    //display the character
    VGA_display_char(c);

    // //restore the position
    vga_buf_cur = current_pos;

    //restore fg and bg
    setColor(color);
    setBackgroundColor(bcolor);

    vga_scroll_disabled = 0;
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

    printk("printk color test: ");

    char *colorWords[16] = {
	"BLACK", "BLUE", "GREEN", "CYAN",
	"RED", "MAGENTA", "BROWN", "GRAY",
  	"DARK_GRAY", "BRIGHT_BLUE", "BRIGHT_GREEN", "BRIGHT_CYAN",
  	"BRIGHT_RED", "BRIGHT_MAGENTA", "YELLOW", "WHITE"
    };
    int colorIdx;
    for(colorIdx = 0; colorIdx < 16; colorIdx++){
	setColor(colorIdx);
	printk(colorWords[colorIdx]);
	setColor(VGA_GRAY);
	printk("-");
    }
    printk("\nbackground color test: ");
    for(colorIdx = 0; colorIdx < 16; colorIdx++){
	setBackgroundColor(colorIdx);
	printk(colorWords[colorIdx]);
	setBackgroundColor(VGA_BLACK);
	printk("-");
    }
    printk("\n");

}
