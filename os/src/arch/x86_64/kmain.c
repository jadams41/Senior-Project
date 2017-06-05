#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "idt.h"
#include "serial.h"
#include "parseMultiboot.h"
#include "memoryManager.h"
#include "test.h"
#include "utils.h"
#include "process.h"
#include "snakes.h"

extern void perform_syscall(int);
extern void load_page_table(uint64_t);
extern void store_control_registers();
extern uint64_t saved_cr2;
extern uint64_t saved_cr3;
extern uint64_t vga_buf_cur;


void test(void *param){
    printk("test\n");
    yield();
    printk("test again\n");
    kexit();
}

void otherTest(void *param){
    printk("boooom\n");
    yield();
    printk("boooom again\n");
    kexit();
}

int kmain(void *multiboot_point, unsigned int multitest){
  asm("cli");

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
  IRQ_clear_mask(KEYBOARD_IRQ);
  IRQ_clear_mask(SERIAL_COM1_IRQ);

  //turn on interrupts
  asm("sti");

  //initialize serial output
  SER_init();
  printk("----SERIAL DEBUGGING BEGIN----\n");

  //initialize the memory info datastructure
  //todo, should probably move this into a local variable inside of memoryManager
  init_usable_segment_struct();

  //take the multiboot_point from entering long mode, and figure out where tags BEGIN
  TagStructureInfo *tagStructureInfo = (TagStructureInfo*)multiboot_point;
  GenericTagHeader *curTag = (GenericTagHeader*)((uint64_t)tagStructureInfo + 8);

  //iterate over and parse all multiboot tags
  while(curTag){
      potentiallyUseTag(curTag);
    //   printTagInfo(curTag);
      curTag = getNextTag(curTag);
  }


  store_control_registers();
  uint64_t ****new_page_table = init_page_table();
  load_page_table((uint64_t)new_page_table);

  init_kheap();

  // setup_snakes(1);
  // PROC_run();
  // printk("done with those snakes!\n");

  // int enabled = 0;
  // while(!enabled) ;


  while(1) asm("hlt");

  return 0;
}
