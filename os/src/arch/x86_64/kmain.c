#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "idt.h"
#include "serial.h"
#include "parseMultiboot.h"
#include "memoryManager.h"

extern void interrupt_test_wrapper();

memory_info memInfo;

void fillPageFrame(void *pf){
    char *testStr = "abcdefghijklmnop";
    int i, j;
    char *currentPositionInsideFrame = (char*)pf;
    for(i = 0; i < 256; i++){
        for(j = 0; j < 16; j++){
            *currentPositionInsideFrame = testStr[j];
            currentPositionInsideFrame += 1;
        }
    }
}

void checkPageFrame(void *pf){
    char *testStr = "abcdefghijklmnop";
    int i,j;
    char *currentPositionInsideFrame = (char*)pf;
    for(i = 0; i < 256; i++){
        for(j = 0; j < 16; j++){
            if(*currentPositionInsideFrame != testStr[j]){
                printk("ERR: frame %lx corrupted\n", pf);
                return;
            }
            currentPositionInsideFrame += 1;
        }
    }
}

void page_frame_allocation_test(){
    void *pf1, *pf2, *pf3;
    printk("frame allocation test\n");
    pf1 = MMU_pf_alloc(&memInfo);
    pf2 = MMU_pf_alloc(&memInfo);
    printk("Allocated 2 pages %lx and %lx\n", pf1, pf2);

    MMU_pf_free(&memInfo, pf2);
    printk("Freed %lx\n", pf2);

    pf3 = MMU_pf_alloc(&memInfo);
    printk("Allocated new page %lx\n", pf3);

    MMU_pf_free(&memInfo, pf1);
    MMU_pf_free(&memInfo, pf3);

    printk("going to attempt to used all of the pages\n");
    int numberOfPagesAllocated = 0;
    while(1){
        pf1 = MMU_pf_alloc(&memInfo);
        if(pf1 == 0){
            break;
        }
        fillPageFrame((void*)pf1);
        numberOfPagesAllocated++;
    }
    printk("was able to allocate %d pages\n", numberOfPagesAllocated);
    printk("going to test the page frames now\n");
    frame_list_node *walker = memInfo.used_frames_list;

    numberOfPagesAllocated = 0;
    while(walker != 0){
        checkPageFrame((void*)walker->beg_addr);
        walker = walker->next_frame;
        numberOfPagesAllocated++;
    }
    // int enabled = 0;
    // while(!enabled) ;
    printk("checked %d pages\n", numberOfPagesAllocated);
    pf1 = (void*)memInfo.used_frames_list->beg_addr;
    char *pageAsString = (char*)pf1;
    pageAsString[4095] = 0;
    printk("%s", pageAsString);
}
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

  //initialize interrupts
  idt_init();
  IRQ_clear_mask(KEYBOARD_IRQ);
  IRQ_clear_mask(SERIAL_COM1_IRQ);

  //turn on interrupts
  asm("sti");

  SER_init();

  printk("----SERIAL DEBUGGING BEGIN----\n");

  init_usable_segment_struct(&memInfo);

  TagStructureInfo *tagStructureInfo = (TagStructureInfo*)multiboot_point;
  uint32_t tagSize = getTotalTagSize(tagStructureInfo);
  printk("The total tag size is %d\n", tagSize);
  GenericTagHeader *curTag = (GenericTagHeader*)((uint64_t)tagStructureInfo + 8);

  printk("going to try starting the next tag at %x\n", curTag);

  enabled = 1;
  while(!enabled) ;

  while(curTag){
    //   printTagInfo(curTag);
      potentiallyUseTag(curTag, &memInfo);
      curTag = getNextTag(curTag);
  }

  printk("done parsing!\n");

  for(enabled = 0; enabled < 25; enabled++){
      printk("\n");
  }

  page_frame_allocation_test();
  // int i;
  // for(i = 0; i < 10; i ++){
  //     void *currentFrame = MMU_pf_alloc(&memInfo);
  //     printk("current frame is at %lx\n", currentFrame);
  //     printk("free list currently points to %lx\n", memInfo.free_frames_list);
  //     printk("used list currently points to %lx\n", memInfo.used_frames_list);
  // }
  //
  // frame_list_node *walker = memInfo.free_frames_list;
  // printk("here is what the free list looks like\n");
  //
  // i = 1;
  // while(walker != 0){
  //     printk("%d) at addr %lx, points to frame at: %lx, next frame node is at %lx, prev: %lx\n", i, walker, walker->beg_addr, walker->next_frame, walker->prev_frame);
  //     walker = walker->next_frame;
  //     i++;
  // }
  //
  // walker = memInfo.used_frames_list;
  // printk("here is what the used list looks like\n");
  //
  // i = 1;
  // while(walker != 0){
  //     printk("%d) at addr %lx, points to frame at: %lx, next frame node is at %lx, prev: %lx\n", i, walker, walker->beg_addr, walker->next_frame, walker->prev_frame);
  //     walker = walker->next_frame;
  //     i++;
  // }
  //
  // printk("freeing all of the used frames\n");
  //
  // while(memInfo.used_frames_list != 0){
  //     MMU_pf_free(&memInfo, (void*)memInfo.used_frames_list->beg_addr);
  //     printk("freed %lx", memInfo.used_frames_list->beg_addr);
  // }
  //
  // printk("here is what the free list looks like\n");
  // walker = memInfo.free_frames_list;
  // i = 1;
  // while(walker != 0){
  //     printk("%d) at addr %lx, points to frame at: %lx, next frame node is at %lx, prev: %lx\n", i, walker, walker->beg_addr, walker->next_frame, walker->prev_frame);
  //     walker = walker->next_frame;
  //     i++;
  // }

  return 0;

}
