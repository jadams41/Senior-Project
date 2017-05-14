#include <stdint-gcc.h>
#include "test.h"
#include "printk.h"
#include "memoryManager.h"

/********** serial tests **********/
//this test writes 320 of the same character to both the local output
//and the serial port, used to track down erroneous characters being printed to serial
void ser_debug(){
    int i;
    char c = 'a';
    for(i= 0; i < 1000; i++){
        if( i % 320 == 0 ){
            c++;
        }
        printk("%c", c);
    }
}

/********** memory manager tests **********/
static void fillPageFrame(void *pf){
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

static void checkPageFrame(void *pf){
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

void page_frame_allocation_test(memory_info *memInfo){
    void *pf1, *pf2, *pf3;
    printk("frame allocation test\n");
    pf1 = MMU_pf_alloc(memInfo);
    pf2 = MMU_pf_alloc(memInfo);
    printk("Allocated 2 pages %lx and %lx\n", pf1, pf2);

    MMU_pf_free(memInfo, pf2);
    printk("Freed %lx\n", pf2);

    pf3 = MMU_pf_alloc(memInfo);
    printk("Allocated new page %lx\n", pf3);

    MMU_pf_free(memInfo, pf1);
    MMU_pf_free(memInfo, pf3);

    printk("going to attempt to used all of the pages\n");
    int numberOfPagesAllocated = 0;
    while(1){
        pf1 = MMU_pf_alloc(memInfo);
        if(pf1 == 0){
            break;
        }
        fillPageFrame((void*)pf1);
        numberOfPagesAllocated++;
    }
    printk("was able to allocate %d pages\n", numberOfPagesAllocated);
    printk("going to test the page frames now\n");
    frame_list_node *walker = memInfo->used_frames_list;

    numberOfPagesAllocated = 0;
    while(walker != 0){
        checkPageFrame((void*)walker->beg_addr);
        walker = walker->next_frame;
        numberOfPagesAllocated++;
    }

    printk("checked %d pages\n", numberOfPagesAllocated);
    pf1 = (void*)memInfo->used_frames_list->beg_addr;
    char *pageAsString = (char*)pf1;
    pageAsString[4095] = 0;
    printk("%s", pageAsString);
}
