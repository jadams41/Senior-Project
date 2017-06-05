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
      printTagInfo(curTag);
      curTag = getNextTag(curTag);
  }


  // page_frame_allocation_test();
  store_control_registers();
  printk("cr2=%lx, cr3=%lx\n", saved_cr2, saved_cr3);
  uint64_t ****new_page_table = init_page_table();
  // check_page_table((void*)new_page_table);

  load_page_table((uint64_t)new_page_table);

  store_control_registers();
  printk("cr2=%lx, cr3=%lx\n", saved_cr2, saved_cr3);

  printk("new page table is at %lx\n", new_page_table);
  // check_page_table((void*)new_page_table);
  // printkTest();
  init_kheap();

  printk_err("sample error\n");

  printk_warn("sample warning\n");
  // virtual_page_frame_test();

  // PROC_create_kthread(test, 0);
  // PROC_create_kthread(otherTest, 0);

  // setup_snakes(1);

  // int enabled = 0;
  // while(!enabled) ;

  // PROC_run();

  void *first = MMU_alloc_pages(5);
  void *second = MMU_alloc_user_pages(5);
  void *third = MMU_alloc_pages(5);
  void *fourth = MMU_alloc_user_pages(5);
  printk("here they are %lx %lx %lx %lx\n", first, second, third, fourth);

  while(1) asm("hlt");

  // void *fiveNewPages = MMU_alloc_pages(5);
  // for(enabled = 0; enabled < 5; enabled++){
  //     printk("page current is at %lx\n", fiveNewPages + enabled);
  // }
  //
  // fiveNewPages = MMU_alloc_pages(5);
  // for(enabled = 0; enabled < 5; enabled++){
  //     printk("page current is at %lx\n", fiveNewPages + enabled);
  // }

  // uint64_t val = page_current[0];
  // printk("%d\n", val);

  return 0;
}
