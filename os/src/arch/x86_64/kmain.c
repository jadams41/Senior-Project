#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "idt.h"

extern void interrupt_test_wrapper();

//this function will poll the keyboard until CTRL-D is pressed
//at this point it will stop polling and print a message indicating this
// void endlessly_poll_keyboard(){
// 	while(1){
//   		char c = getchar();

//   		//check for EOF (-1)
//   		if(c < 0) break;
//   		if(c){
//   			// printk("%hx\n", uc);
//   			VGA_display_char(c);
//   		}
//   	}
//   	printk("\n[DONE POLLING]\n");
// }

// void breakpoint(){
//   printk("at a breakpoint\n");
// }

int kmain(){
  asm("cli");
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
  // VGA_display_char('\n');
  // printk("about to init ps2\n");

  // initPs2();
  // printk("ps2 initialized\n");

  // printk("about to configure keyboard\n");
  // keyboard_config();
  // printk("keyboard_configured\n");

//  endlessly_poll_keyboard();


  printk("[ABOUT TO INIT IDT]\n");

  idt_init();
  //printk("[IDT INITIALIZED\n");
  
  //printk("[ABOUT TO ENABLE KEYBOARD INTERRUPTS]\n");
  kb_init();
  asm("sti");
  //printk("[KEYBOARD INTERRUPT INITIALIZED\n");
  // enabled = 0;
  // while (!enabled) ;
 // while(!enabled) ;

  while(!enabled) ;

  //try INTERRUPTS
  interrupt_test_wrapper();

  // breakpoint();

  return 0;

}