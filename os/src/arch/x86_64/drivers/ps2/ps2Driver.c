#include "ps2Driver.h"
#include "../../utils/printk.h"
#include <stdint-gcc.h>
#include "../../utils/utils.h"
#include "keyboard.h"

#define PS2_CMD_CONF 0x20
#define PS2_DATA 0x60
#define PS2_CMD 0x64
#define PS2_STATUS PS2_CMD
#define PS2_STATUS_OUTPUT 1
#define PS2_STATUS_INPUT (1 << 1)
#define CMD_PORT 0x64
#define DATA_PORT 0x60
#define BUFF_SIZE 10

uint8_t left_shift_pressed = 0;
uint8_t right_shift_pressed = 0;

uint8_t left_ctrl_pressed = 0;
uint8_t right_ctrl_pressed = 0;
extern int stupidFunctionDead;

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

//todo - this could probably be removed
char getCharFromScan(int scan){
	return scancode_dict[scan];
}

//todo - move these structures to the header file
//todo - this really needs to be sanitized
typedef struct {
    uint8_t output_buf:1;
    uint8_t input_buf:1;
    uint8_t sys_flag:1;
    uint8_t cmd_data:1;
    uint8_t unkwn_1:1;
    uint8_t unkwn_2:1;
    uint8_t time_err:1;
    uint8_t parity_err:1;
} __attribute__((packed)) status;

//todo - this really needs to be sanitized
typedef struct {
    uint8_t first_int:1;
    uint8_t second_int:1;
    uint8_t sys_flag:1;
    uint8_t zero_2:1;
    uint8_t first_clk:1;
    uint8_t second_clk:1;
    uint8_t first_trans:1;
    uint8_t zero_1:1;
} __attribute__((packed)) config;

//todo - this really needs to be sanitized
void initPs2() {
    config *cfg;
    uint8_t retVal;
    //initScancodes();
    pollInputBuffer();
    outb(CMD_PORT, 0xAD);
    pollInputBuffer();
    outb(CMD_PORT, 0xA7);
    pollInputBuffer();
    outb(CMD_PORT, 0x20);
    retVal = pollOutputBuffer();
    cfg = (config*)&retVal;
    cfg->first_int = 0;
    cfg->second_int = 0;
    cfg->first_trans = 0;
    outb(CMD_PORT, 0x60);
    pollInputBuffer();
    outb(DATA_PORT, retVal);
    outb(CMD_PORT, 0xAE);
    pollInputBuffer();
    outb(CMD_PORT, 0xA8);
    pollInputBuffer();
    outb(CMD_PORT, 0x60);
    cfg->first_int = 1;
    cfg->first_clk = 1;
    cfg->second_int = 1;
    cfg->second_clk = 1;
    outb(DATA_PORT, retVal);
    pollInputBuffer();
    outb(DATA_PORT, 0xFF);
    pollInputBuffer();
    outb(DATA_PORT, 0xF0);
    pollInputBuffer();
    outb(DATA_PORT, 0x02);
    pollInputBuffer();
    outb(DATA_PORT, 0xF4);
}

void pollInputBuffer() {
    status *stat;
    uint8_t retVal;
    retVal = inb(CMD_PORT);
    stat = (status*)&retVal;
    while(stat->input_buf || stat->time_err) {
        retVal = inb(CMD_PORT);
    }
}

int8_t pollOutputBuffer() {
    status *stat;
    int8_t retVal;
    retVal = inb(CMD_PORT);
    stat = (status*)&retVal;
    while(!(stat->output_buf) || stat->time_err) {
        retVal = inb(CMD_PORT);
    }
    return inb(DATA_PORT);
}

// char getScancode() {
//     char val = '\0';
//     while(1) {
//     	block_until_response_available();
//         val = inb(PS2_DATA);

//         //means that key was pressed
//         if(val > 0){
//         	//shift was pressed
//         	if(val == 0x2a){
//         		left_shift_pressed = 1;
//         	}
//         	else if(val == 0x36){
//         		right_shift_pressed = 1;
//         	}
//         	else if(val == 0x1d){
//         		left_ctrl_pressed = 1;
//         	}
//         	else if(val == 0xe0){
//         		right_ctrl_pressed = 1;
//         	}
//         	else {
// 		        return val;
//         	}
//         }
//         //means that key was released
//         else {
//         	unsigned char uval = (unsigned char)val;
//         	if(uval == 0xaa){
//         		left_shift_pressed = 0;
//         	}
//         	else if(uval == 0xb6){
//         		right_shift_pressed = 0;
//         	}
//         	// else if(uval == 0x)
//         	else if(uval == 0x9d){
//         		left_ctrl_pressed = 0;
//         	}
//         }
//     }
// }

// char getchar(){
// 	// return getScancode();
// 	char res = scancode_dict[(unsigned int)getScancode()];
// 	//check for EOF (CTRL+D)
// 	if(res == 'd' && (left_ctrl_pressed || right_ctrl_pressed))
// 		return -1;
// 	return left_shift_pressed || right_shift_pressed ? shift_down_dict[(unsigned int)res] : res;
// }

// void ps2_init(){
// 	ps2_config* ps2conf;
// 	uint8_t inbRes;

// 	//disable devices on ch1 & ch2
// 	block_until_input_ready();
// 	outb(0xAD, PS2_CMD);

// 	block_until_input_ready();
// 	outb(0xA7, PS2_CMD);

// 	//read the PS/2 config byte
// 	block_until_input_ready();
// 	outb(0x20, PS2_CMD);

// 	block_until_response_available();
// 	inbRes = inb(PS2_DATA);
// 	ps2conf = (ps2_config*)&inbRes;

// 	//enable the clock on ch1
// 	ps2conf->port1clk = 1;
// 	//enable interrupts on ch1
// 	ps2conf->interrupt1 = 1;

// 	//disable port2 clk and inter
// 	ps2conf->port2clk = 0;
// 	ps2conf->interrupt2 = 0;

// 	//write config byte back out to the PS/2 controller
// 	outb(0x60, PS2_CMD);
// 	block_until_input_ready();
// 	outb(inbRes, PS2_DATA);

// 	outb(0xAE, PS2_CMD);
// 	block_until_input_ready();

// 	outb(0xA8, PS2_CMD);
// 	block_until_input_ready();

// 	outb(0x60, PS2_CMD);
// 	ps2conf->interrupt1 = 1;
// 	ps2conf->port1clk = 0;
// 	ps2conf->interrupt2 = 0;
// 	ps2conf->port2clk = 1;

// 	outb(inbRes, PS2_DATA);
// 	block_until_input_ready();
// 	outb(0xFF, PS2_DATA);
// 	block_until_input_ready();
// 	outb(0xF0, PS2_DATA);
// 	block_until_input_ready();
// 	outb(0x02, PS2_DATA);
// 	block_until_input_ready();
// 	outb(0xF4, PS2_DATA);
// }

// void keyboard_config(){
// 	//reset the keyboard
// 	outb(PS2_DATA, 0xFE);
// 	inb(PS2_STATUS);

// 	//set the keyboard to a known scan code
// 	outb(PS2_DATA, 0xF0);
// 	inb(PS2_STATUS);

// 	outb(PS2_DATA, 1);
// 	inb(PS2_STATUS);

// 	//enable the keyboard
// 	outb(PS2_DATA, 0xF4);
// }

// /**
//   * poll status until the 0 bit in response is set
//   * response is ready when this is set
//   */
// void block_until_response_available(){
// 	unsigned char status = inb(PS2_STATUS);
// 	struct ps2_status* status_mask = (struct ps2_status*)&status;

// 	while(!(status_mask->out_buf_status) || status_mask->time_out_err){
// 		status = inb(PS2_STATUS);
// 	}

// }

/**
  * poll status until the 1 bit in response is clear
  * this is needed before sending the next byte of a two byte command
  */
// void block_until_input_ready(){
// 	unsigned char status = inb(PS2_STATUS);
// 	struct ps2_status *mask = (struct ps2_status*)&status;

// 	while (mask->out_buf_status && !mask->time_out_err){
// 		status = inb(PS2_STATUS);
// 	}
// }

// int tryToTriggerPageFault(){
// 	int *fakePtr = (int *)0xffffffffff;
// 	int test = *fakePtr;
// 	return test;
// }

void keyboard_handler_main(char scan){
	/* write End Of Input */
	outb(0x20, 0x20);

	// tryToTriggerPageFault();

	//translate the scancode to the ascii char
	//char val = scancode_dict[(int)scan];

	if(scan > 0){
    	//shift was pressed
    	if(scan == 0x2a){
    		left_shift_pressed = 1;
    	}
    	else if(scan == 0x36){
    		right_shift_pressed = 1;
    	}
    	else if(scan == 0x1d){
    		left_ctrl_pressed = 1;
    	}
    	else if(scan == 0xe0){
    		right_ctrl_pressed = 1;
    	}
    	else {
    		char val = scancode_dict[(uint8_t)scan];
			if(left_ctrl_pressed || right_ctrl_pressed){
				if(val == 'c'){
					stupidFunctionDead = 1;
					return;
				}
			}
			uint8_t idx = (uint8_t)val;
			// printk("%c", left_shift_pressed || right_shift_pressed ? shift_down_dict[idx] : val);
			KBD_write(left_shift_pressed || right_shift_pressed ? shift_down_dict[idx] : val);
		}
    }
    // means that key was released
    else {
    	unsigned char uval = (unsigned char)scan;
    	if(uval == 0xaa){
    		left_shift_pressed = 0;
    	}
    	else if(uval == 0xb6){
    		right_shift_pressed = 0;
    	}
    	// else if(uval == 0x)
    	else if(uval == 0x9d){
    		left_ctrl_pressed = right_ctrl_pressed = 0;
    	}
    }


}

/*
 * this function merely handles the sent scancode
 * meant to handle a given scancode regardless of polling or interrupt
 * driven keyboard I/O
 */
void handle_generic_keypress(char scancode){

}
