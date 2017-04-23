#include "ps2Driver.h"
#include <stdint-gcc.h>

#define PS2_CMD_CONF 0x20
#define PS2_DATA 0x60
#define PS2_CMD 0x64
#define PS2_STATUS PS2_CMD
#define PS2_STATUS_OUTPUT 1
#define PS2_STATUS_INPUT (1 << 1)

uint8_t left_shift_pressed = 0;
uint8_t right_shift_pressed = 0;

uint8_t left_ctrl_pressed = 0;
uint8_t right_ctrl_pressed = 0;

char scancode_dict[256];
char shift_down_dict[256];

void initialize_scancodes(){
	scancode_dict[2] = '1';
	scancode_dict[3] = '2';
	scancode_dict[4] = '3';
	scancode_dict[5] = '4';
	scancode_dict[6] = '5';
	scancode_dict[7] = '6';
	scancode_dict[8] = '7';
	scancode_dict[9] = '8';
	scancode_dict[10] = '9';
	scancode_dict[11] = '0';
	scancode_dict[12] = '-';
	scancode_dict[13] = '=';
	scancode_dict[14] = 8; //backspace
	scancode_dict[16] = 'q';
	scancode_dict[17] = 'w';
	scancode_dict[18] = 'e';
	scancode_dict[19] = 'r';
	scancode_dict[20] = 't';
	scancode_dict[21] = 'y';
	scancode_dict[22] = 'u';
	scancode_dict[23] = 'i';
	scancode_dict[24] = 'o';
	scancode_dict[25] = 'p';
	scancode_dict[26] = '[';
	scancode_dict[27] = ']';
	scancode_dict[28] = '\n';
	scancode_dict[29] = 'x';
	scancode_dict[30] = 'a';
	scancode_dict[31] = 's';
	scancode_dict[32] = 'd';
	scancode_dict[33] = 'f';
	scancode_dict[34] = 'g';
	scancode_dict[35] = 'h';
	scancode_dict[36] = 'j';
	scancode_dict[37] = 'k';
	scancode_dict[38] = 'l';
	scancode_dict[39] = ';';
	scancode_dict[40] = '\'';
	scancode_dict[41] = '\\';
	scancode_dict[42] = 14; //left shift key
	scancode_dict[43] = 'x';
	scancode_dict[44] = 'z';
	scancode_dict[45] = 'x';
	scancode_dict[46] = 'c';
	scancode_dict[47] = 'v';
	scancode_dict[48] = 'b';
	scancode_dict[49] = 'n';
	scancode_dict[50] = 'm';
	scancode_dict[51] = ',';
	scancode_dict[52] = '.';
	scancode_dict[53] = '/';
	scancode_dict[54] = 14; //left shift key	
	scancode_dict[57] = ' ';
}

void initialize_shift_down_dict(){
	shift_down_dict['`'] = '~';
	shift_down_dict['1'] = '!';
	shift_down_dict['2'] = '@';
	shift_down_dict['3'] = '#';
	shift_down_dict['4'] = '$';
	shift_down_dict['5'] = '%';
	shift_down_dict['6'] = '^';
	shift_down_dict['7'] = '&';
	shift_down_dict['8'] = '*';
	shift_down_dict['9'] = '(';
	shift_down_dict['0'] = ')';
	shift_down_dict['-'] = '_';
	shift_down_dict['='] = '+';
	shift_down_dict['q'] = 'Q';
	shift_down_dict['w'] = 'W';
	shift_down_dict['e'] = 'E';
	shift_down_dict['r'] = 'R';
	shift_down_dict['t'] = 'T';
	shift_down_dict['y'] = 'Y';
	shift_down_dict['u'] = 'U';
	shift_down_dict['i'] = 'I';
	shift_down_dict['o'] = 'O';
	shift_down_dict['p'] = 'P';
	shift_down_dict['['] = '{';
	shift_down_dict[']'] = '}';
	shift_down_dict['\\'] = '|';
	shift_down_dict['a'] = 'A';
	shift_down_dict['s'] = 'S';
	shift_down_dict['d'] = 'D';
	shift_down_dict['f'] = 'F';
	shift_down_dict['g'] = 'G';
	shift_down_dict['h'] = 'H';
	shift_down_dict['j'] = 'J';
	shift_down_dict['k'] = 'K';
	shift_down_dict['l'] = 'L';
	shift_down_dict[';'] = ':';
	shift_down_dict['\''] = '"';
	shift_down_dict['z'] = 'Z';
	shift_down_dict['x'] = 'X';
	shift_down_dict['c'] = 'C';
	shift_down_dict['v'] = 'V';
	shift_down_dict['b'] = 'B';
	shift_down_dict['n'] = 'N';
	shift_down_dict['m'] = 'M';
	shift_down_dict[','] = '<';
	shift_down_dict['.'] = '>';
	shift_down_dict['/'] = '?';
}

// extern void outb(unsigned char value, unsigned short int port);
// extern unsigned char inb(unsigned short int port);

void inline outb(uint8_t val, uint16_t port) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inline inb(uint16_t port) {
    uint8_t val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

struct ps2_config {
	unsigned int interrupt1:1;
	unsigned int interrupt2:1;
	unsigned int systemFlag:1;
	unsigned int zero:1;
	unsigned int port1clk:1;
	unsigned int port2clk:1;
	unsigned int translation:1;
	unsigned int zero2:1;
} __attribute__((packed));

struct ps2_status {
	unsigned int out_buf_status:1;
	unsigned int in_buf_status:1;
	unsigned int sys_flag:1;
	unsigned int cmd_or_data:1;
	unsigned int unkn1:1;
	unsigned int unkn2:1;
	unsigned int time_out_err:1;
	unsigned int parity_err:1;
} __attribute__((packed));

typedef struct ps2_config ps2_config;

char getScancode() {
    char val = '\0';
    while(1) {
    	block_until_response_available();
        val = inb(PS2_DATA);

        //means that key was pressed
        if(val > 0){
        	//shift was pressed
        	if(val == 0x2a){
        		left_shift_pressed = 1;
        	}
        	else if(val == 0x36){
        		right_shift_pressed = 1;
        	}
        	//todo add checking for ctrl
        	else {
		        return val;
        	}
        }
        //means that key was released
        else {
        	unsigned char uval = (unsigned char)val;
        	if(uval == 0xaa){
        		left_shift_pressed = 0;
        	}
        	else if(uval == 0xb6){
        		right_shift_pressed = 0;
        	}
        	//todo add more cases for ctrl and such
        }
    }
}

char getchar(){
	// return getScancode();
	char res = scancode_dict[getScancode()];
	
	return left_shift_pressed || right_shift_pressed ? shift_down_dict[res] : res;
}

void ps2_init(){
	ps2_config* ps2conf;
	unsigned char inbRes;

	//disable devices on ch1 & ch2
	block_until_input_ready();
	outb(0xAD, PS2_CMD);

	block_until_input_ready();
	outb(0xA7, PS2_CMD);

	//read the PS/2 config byte
	block_until_input_ready();
	outb(0x20, PS2_CMD);

	block_until_response_available();
	inbRes = inb(PS2_DATA);
	ps2conf = (ps2_config*)&inbRes;

	//enable the clock on ch1
	ps2conf->port1clk = 1;
	//enable interrupts on ch1
	ps2conf->interrupt1 = 1;

	//disable port2 clk and inter
	ps2conf->port2clk = 0;
	ps2conf->interrupt2 = 0;

	//write config byte back out to the PS/2 controller
	outb(0x60, PS2_CMD);
	block_until_input_ready();
	outb(inbRes, PS2_DATA);

}

void keyboard_config(){
	unsigned char response;

	//reset the keyboard
	outb(PS2_DATA, 0xFE);
	response = inb(PS2_STATUS);

	//set the keyboard to a known scan code
	outb(PS2_DATA, 0xF0);
	response = inb(PS2_STATUS);

	outb(PS2_DATA, 1);
	response = inb(PS2_STATUS);

	//enable the keyboard
	outb(PS2_DATA, 0xF4);
}

/**
  * poll status until the 0 bit in response is set
  * response is ready when this is set
  */
void block_until_response_available(){
	unsigned char status = inb(PS2_STATUS);
	struct ps2_status* status_mask = (struct ps2_status*)&status;

	while(!(status_mask->out_buf_status) || status_mask->time_out_err){
		status = inb(PS2_STATUS);
	}

}

/**
  * poll status until the 1 bit in response is clear
  * this is needed before sending the next byte of a two byte command
  */
void block_until_input_ready(){
	unsigned char status = inb(PS2_STATUS);
	struct ps2_status *mask = (struct ps2_status*)&status;

	while (mask->out_buf_status && !mask->time_out_err){
		status = inb(PS2_STATUS);
	}	
}