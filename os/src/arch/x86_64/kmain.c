#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "idt.h"
#include "serial.h"

extern void interrupt_test_wrapper();

int kmain(){
  asm("cli");
  int enabled = 0;

  //set up and test the VGA
  VGA_clear();
  vgaDispCharTest();
  VGA_display_str("string print test\n");

  //set up the things needed for printk
  initialize_scancodes();
  initialize_shift_down_dict();
  disableSerialPrinting();

  //currently working without this, too scared to uncomment
  // initPs2();
  // keyboard_config();

  //initialize interrupts
  idt_init();
  kb_init();

  //turn on interrupts
  asm("sti");

  //initialize serial tx interrupts and writing
  SER_init();
  enableSerialPrinting();
  SER_write("----SERIAL DEBUGGING BEGIN----\n",31);

  //wait
  while(!enabled) ;

  return 0;

}