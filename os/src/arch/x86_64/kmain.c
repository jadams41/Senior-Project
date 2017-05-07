#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "idt.h"
#include "serial.h"
#include "parseMultiboot.h"

extern void interrupt_test_wrapper();

int kmain(void *multiboot_point, unsigned int multitest){
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

  // printk("multiboot_pointer %X\n", multiboot_point);
  // printk("multiboot_test %X\n", multitest);

  //currently working without this, too scared to uncomment
  // initPs2();
  // keyboard_config();
  // while (!enabled) ;
  // TagStructureInfo *tagStructureInfo = (TagStructureInfo*)multiboot_point;
  // uint32_t tagSize = getTotalTagSize(tagStructureInfo);
  // printk("The total tag size is %d\n", tagSize);
  // GenericTagHeader *curTag = (GenericTagHeader*)((uint64_t)tagStructureInfo + 8);
  //
  // printk("going to try starting the next tag at %x\n", curTag);
  // while(curTag){
  //     printTagInfo(curTag);
  //     curTag = getNextTag(curTag);
  // }
  //initialize interrupts
  idt_init();
  IRQ_clear_mask(KEYBOARD_IRQ);
  IRQ_clear_mask(SERIAL_COM1_IRQ);

  //turn on interrupts
  asm("sti");

  //initialize serial tx interrupts and writing
  SER_init();
  // enableSerialPrinting();

  printk("----SERIAL DEBUGGING BEGIN----\n");

  // printkTest();

  interrupt_test_wrapper();

  while(!enabled) ;


  return 0;

}
