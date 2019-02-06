#include <stdint-gcc.h>
#include "test.h"
#include "utils/printk.h"
#include "drivers/memory/memoryManager.h"

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

static void __attribute__((unused)) checkPageFrame(void *pf){
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
    pf1 = MMU_pf_alloc();
    pf2 = MMU_pf_alloc();
    printk("Allocated 2 pages %lx and %lx\n", pf1, pf2);

    MMU_pf_free(pf2);
    printk("Freed %lx\n", pf2);

    pf3 = MMU_pf_alloc();
    printk("Allocated new page %lx\n", pf3);

    MMU_pf_free(pf1);
    MMU_pf_free(pf3);

    printk("going to attempt to used all of the pages\n");
    int numberOfPagesAllocated = 0;
    while(1){
        pf1 = MMU_pf_alloc();
        if(pf1 == 0){
            break;
        }
        fillPageFrame((void*)pf1);
        numberOfPagesAllocated++;
    }
    printk("was able to allocate %d pages\n", numberOfPagesAllocated);
    // printk("going to test the page frames now\n");
    // frame_list_node *walker = memInfo->used_frames_list;
    //
    // numberOfPagesAllocated = 0;
    // while(walker != 0){
    //     checkPageFrame((void*)walker->beg_addr);
    //     walker = walker->next_frame;
    //     numberOfPagesAllocated++;
    // }
    //
    // printk("checked %d pages\n", numberOfPagesAllocated);
    // pf1 = (void*)memInfo->used_frames_list->beg_addr;
    // char *pageAsString = (char*)pf1;
    // pageAsString[4095] = 0;
    // printk("%s", pageAsString);
}

/***** Virtual Page Frame Allocation Test *****/
void virtual_page_frame_test(){
    printk("***************** Virtual Page Frame Alloc Test *****************\n");
    int i;
    void *vpages[10];
    char *pagePointer;

    //allocate virtual pages
    for(i = 0; i < 10; i++){
        vpages[i] = MMU_alloc_page();

        printk("page %d is at %lx\n", i, vpages[i]);
    }

    printk("will now demand pages 0, 5, and 7, should see a message for each of the pages from the page fault handler\n");
    pagePointer = (char*)(vpages[0]);
    *pagePointer = '0';

    pagePointer = (char*)(vpages[5]);
    *pagePointer = '5';

    pagePointer = (char*)(vpages[7]);
    *pagePointer = '7';

    printk("writing a message into page 5, which will be freed and the physical page reused\n");
    pagePointer = (char*)(vpages[5]);
    pagePointer[0] = 'h';
    pagePointer[1] = 'e';
    pagePointer[2] = 'l';
    pagePointer[3] = 'l';
    pagePointer[4] = 'o';
    pagePointer[5] = ' ';
    pagePointer[6] = 'w';
    pagePointer[7] = 'o';
    pagePointer[8] = 'r';
    pagePointer[9] = 'l';
    pagePointer[10] = 'd';
    pagePointer[11] = '!';
    pagePointer[12] = 0;
    printk("message from page 5: %s\n", pagePointer);
    MMU_free_page(vpages[5]);

    pagePointer = (char*)(vpages[8]);
    pagePointer[0] = 'H';

    printk("message from page 8: %s\n", pagePointer);

    printk("will now try to completely exhaust the virtual address space\n");
    int count = 0;
    void *vpage;
    while(1){
        vpage = MMU_alloc_page();
        if(vpage == 0) break;
        count++;
    }
    printk("allocated %d virtual pages before implosion\n", count);
}

void kmalloc_test(){
    printk("***************** Kmalloc Test *****************\n");
    int i;
    char *msg = "kmalloc is working!";
    char *c = kmalloc(sizeof(char) * 20);

    for(i = 0; i < 20; i++){
        c[i] = msg[i];
    }
    printk("here is the message from the malloced (64b) block %s\n", c);
    kfree(c);

    c = kmalloc(sizeof(char) * 20);
    c[0] = 'k';
    c[1] = 'f';
    c[2] = 'r';
    c[3] = 'e';
    c[4] = 'e';
    c[5] = ' ';
    c[6] = ' ';

    printk("here is the message from the 2nd malloc block %s\n", c);

    printk("will now try to exhaust kmalloc\n");
    int count = 0;
    while(1){
        void *mem = kmalloc(10); //32
        if(!mem) break;
        mem = kmalloc(20); //64
        if(!mem) break;
        mem = kmalloc(65); //128
        if(!mem) break;
        mem = kmalloc(129); //512
        if(!mem) break;
        mem = kmalloc(513); //1024
        if(!mem) break;
        mem = kmalloc(1025); //2048

        if(!mem) break;

        count++;
        if(!(count % 10000)){
            printk("able to allocate %d 64b blocks so far\n", count);
        }
    }
    printk("able to allocate %d blocks before exhaustion\n", count * 6);
}
