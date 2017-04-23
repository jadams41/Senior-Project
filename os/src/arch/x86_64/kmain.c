#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"

int kmain(){
  int enabled = 0;

  //clear the vga console now that we are up and running
  VGA_clear();

  vgaDispCharTest();

  VGA_display_str("string print test\n");

  //printkTest();


  //while(!enabled) ;

  initialize_scancodes();
  initialize_shift_down_dict();

  // while(!enabled) ;
  VGA_display_char('\n');
  printk("about to init ps2\n");

  ps2_init();
  printk("ps2 initialized\n");

  printk("about to configure keyboard\n");
  keyboard_config();
  printk("keyboard_configured\n");

  while(1){
  	char c = getchar();
  	if(c){
  		VGA_display_char(c);
  	}
  	// printk("%c\n", c);
  }
}

