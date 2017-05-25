#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "idt.h"
#include "serial.h"
#include "parseMultiboot.h"
#include "memoryManager.h"
#include "test.h"
#include "utils.h"

extern void interrupt_test_wrapper();
extern void load_page_table(uint64_t);
extern void store_control_registers();
extern uint64_t saved_cr2;
extern uint64_t saved_cr3;

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
  printkTest();

  enabled = 0;
  while(!enabled) ;

  void *newPage = MMU_alloc_page();
  uint64_t *page_current = (uint64_t*)newPage;

  //this should page fault because the page is only virtually allocated
  page_current[0] = 'c';
  printk("page current is at %lx\n", page_current);
  printk("page was set correctly? %c\n", page_current[0]);

  char *pageStrs[5];

  for(enabled = 0; enabled < 5; enabled++){
      page_current = (uint64_t*)MMU_alloc_page();
      printk("page current is at %lx\n", page_current);
      char *str_in_page = (char*)page_current;

      pageStrs[enabled] = str_in_page;

      str_in_page[0] = '0' + enabled;
      str_in_page[1] = 't';
      str_in_page[2] = 'e';
      str_in_page[3] = 's';
      str_in_page[4] = 't';
      str_in_page[5] = '0' + enabled;
      printk("%s\n", str_in_page);
  }

  for(enabled = 0; enabled < 5; enabled++){
      printk("%s\n", pageStrs[enabled]);
  }

  page_current = (uint64_t*)pageStrs[2];
  MMU_free_pages(page_current, 2);

  printk("freed from %lx to %lx\n", page_current, page_current + 512 * 3);

  page_current = MMU_alloc_pages(3);
  printk("allocated 3 pages in a row starting at %lx\n", page_current);

  for(enabled = 0; enabled < 5; enabled++){
      page_current = MMU_alloc_page();
      printk("allocated a new page at %lx\n", page_current);
  }

  printk("testing done\n");

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
