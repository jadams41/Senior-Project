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
#include "keyboard.h"
#include "blockDeviceDriver.h"
#include "string.h"
#include "kernel_functions.h"

/* assembly function entry points */
extern void perform_syscall(int);
extern void load_page_table(uint64_t);
extern void store_control_registers();

/* assembly variable references */
extern uint64_t saved_cr2;
extern uint64_t saved_cr3;
extern uint64_t vga_buf_cur;

struct BlockDev *ata = 0;
extern PROC_context *curProc;
int stupidFunctionDead = 0;

void bringupKernel(kernel_bringup_config *conf){
  /* turn off interrupts while setting up kernel */
  asm("cli");

  /* set up and test the VGA */
  VGA_clear();

  /* set up the things needed for printk */
  initialize_scancodes();
  initialize_shift_down_dict();
  disableSerialPrinting();

  /* initialize interrupts */
  idt_init();
  IRQ_clear_mask(KEYBOARD_IRQ);
  IRQ_clear_mask(SERIAL_COM1_IRQ);
  IRQ_clear_mask(SLAVE_PIC_IRQ);
  IRQ_clear_mask(PRIMARY_ATA_CHANNEL_IRQ);
  // IRQ_clear_mask(SECONDARY_ATA_CHANNEL_IRQ);

  /* set up the keyboard state */
  //todo: figure out exactly what this means
  init_kbd_state();

  /* turn on interrupts back on */
  asm("sti");
  
  /* initialize serial output */
  SER_init();
  printk("----SERIAL DEBUGGING BEGIN----\n");

  /* initialize the memory info datastructure */
  //todo, should probably move this into a local variable inside of memoryManager
  init_usable_segment_struct();

  /* take the multiboot_point from entering long mode, 
   * and figure out where tags BEGIN */
  TagStructureInfo *tagStructureInfo = (TagStructureInfo*)conf->multiboot_point;
  GenericTagHeader *curTag = (GenericTagHeader*)((uint64_t)tagStructureInfo + 8);

  /* iterate over and parse all multiboot tags */
  while(curTag){
      potentiallyUseTag(curTag);
      curTag = getNextTag(curTag);
  }

  /* store control registers */
  //todo: figure out why this is needed (pretty sure its important just don't remember why)
  store_control_registers();

  /* set up new page table */
  uint64_t ****new_page_table = init_page_table();
  load_page_table((uint64_t)new_page_table);

  /* initialize kernel heap */
  init_kheap();

  /* create a kernel thread for handling PS/2 interrupts */
  if(conf->enable_keyboard_proc){
    PROC_create_kthread((kproc_t)grabKeyboardInput, (void *)0);
  }
}

void runKernelProcesses(){
  while(1){
    PROC_run();
    asm("hlt");
  }
}

//takes params uint64_t blockNum, void *dst
void readBlock(void *params){
    uint64_t blockNum;
    void *dst;
    if(params == 0){
        printk_err("No parameters were passed to readBlock, nothing will happen\n");
        kexit();
    }
    if(ata == 0){
        printk_err("ata not initialized, nothing will happen\n");
        kexit();
    }
    uint64_t *paramArr = (uint64_t*)params;
    uint64_t paramLen = *paramArr;
    if(paramLen -1 != 2){
        printk_err("passed the wrong number of parameters, expected 2 and received %d", paramLen -1);
        kexit();
    }

    blockNum = paramArr[1];
    dst = (void*)paramArr[2];
    ata->read_block(ata, blockNum, dst);

    printk_info("done reading block %d, creating process to print this block now\n", blockNum);

    uint64_t *printBlockParms = kmalloc(sizeof(uint64_t) * 3);
    printBlockParms[0] = 3;
    printBlockParms[1] = (uint64_t)dst;
    printBlockParms[2] = (uint64_t)blockNum;

    asm("CLI");
    PROC_create_kthread(printBlock, printBlockParms);
    asm("STI");

    kexit();
}

void readBlock0(){
    char dest[512];
    memset(dest, 0, 512);
    ata->read_block(ata, 0, dest);

    printk("done reading block 0!\n");
    int i;
    for(i = 0; i < 512; i += 8){
        printk("%d] %lx\n", i, *((uint64_t*)(dest + i)));
    }
    kexit();
}

void readBlock32(){
    char dest[512];
    memset(dest, 0, 512);
    ata->read_block(ata, 32, dest);

    printk("done reading block 32!\n");
    int i;
    for(i = 0; i < 512; i += 8){
        printk("%d] %lx\n", i, *((uint64_t*)(dest + i)));
    }
    kexit();
}

//takes params void *blockBuffer, uint64_t blockNumber
void printBlock(void *params){
    uint64_t *paramArr = (uint64_t*)params;
    if(params == 0){
        printk_err("No parameters were passed to printBlock, nothing will happen\n");
        kexit();
    }
    uint64_t paramLen = *paramArr;
    if(paramLen != 3){
        printk_err("the wrong number of parameters were passed to printBlock, nothing will happen\n");
        kexit();
    }
    uint8_t *block = (uint8_t*)paramArr[1];
    uint64_t blockNum = paramArr[2];
    uint64_t blockOffset = blockNum * 512;

    printk("---------- BLOCK-%d ----------\n", blockNum);
    int i;
    for(i = 0; i < 512; i+=16){
        int j;
        printk("%06x ", blockOffset + i);
        for(j = 0; j < 16; j+=2){
            printk("%02hx%02hx ", block[i+j+1], block[i+j]);
        }
        printk("\n");
    }
    printk("---------------------------\n");
    kfree(params);
    kexit();
}

void grabKeyboardInput(){
    printk("keyboard listening process is go (PID=%d)\n", curProc->pid);
    while(1){
        printk("%c", KBD_read());
    }
}

//takes params uint16_t base, uint16_t master, uint8_t slave, uint8_t irq
void initBlockDevice(void *params){
    if(params == 0){
        printk_err("did not pass in any parameters to init block device, nothing will happen\n");
        kexit();
    }
    uint64_t *paramArr = params;
    int arrLen = *paramArr;
    if(arrLen - 1 != 4){
        printk_err("passed in the wrong number of parameters, expected 4 and received %d\n", arrLen - 1);
        kexit();
    }
    uint16_t base = paramArr[1];
    uint16_t master = paramArr[2];
    uint8_t slave = paramArr[3];
    uint8_t irq = paramArr[4];

    ata = ata_probe(base, master, slave, irq);
    kexit();
}

