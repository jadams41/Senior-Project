#include <stdint-gcc.h>
#include "utils/printk.h"
#include "drivers/ps2/ps2Driver.h"
#include "drivers/interrupts/idt.h"
#include "drivers/serial/serial.h"
#include "utils/parseMultiboot.h"
#include "drivers/memory/memoryManager.h"
#include "test/test.h"
#include "utils/utils.h"
#include "types/process.h"
#include "test/snakes/snakes.h"
#include "drivers/ps2/keyboard.h"
#include "drivers/block/blockDeviceDriver.h"
#include "types/string.h"
#include "drivers/fs/vfs.h"
#include "drivers/fs/fat32.h"
#include "drivers/pci/pci.h"
#include "drivers/net/8139too.h"

extern void perform_syscall(int);
extern void load_page_table(uint64_t);
extern void store_control_registers();
extern uint64_t saved_cr2;
extern uint64_t saved_cr3;
extern uint64_t vga_buf_cur;

int stupidFunctionDead = 0;
extern PROC_context *curProc;
struct BlockDev *ata = 0;
struct PCIDevice *pci_devices = NULL;

// configuration globals
int no_debug = 0;
enum printing_level {
    info, //print everything
    warn, //exclude info
    err, //exclude info and warn
    silent
} printk_conf = info;


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

    /* printk("---------- BLOCK-%d ----------\n", blockNum); */
    int i;
    for(i = 0; i < 512; i+=16){
        int j;
        printk("%06x ", blockOffset + i);
        for(j = 0; j < 16; j+=2){
            printk("%02hx%02hx ", block[i+j+1], block[i+j]);
        }
        printk("\n");
    }
    /* printk("---------------------------\n"); */
    kfree(params);
    kexit();
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
        printk_err("passed the wrong number of parameters, expected 2 and received %llu", paramLen -1);
        kexit();
    }

    blockNum = paramArr[1];
    dst = (void*)paramArr[2];
    ata->read_block(ata, blockNum, dst);

    /* printk_info("done reading block %d, creating process to print this block now\n", blockNum); */

    uint64_t *printBlockParms = kmalloc(sizeof(uint64_t) * 3);
    printBlockParms[0] = 3;
    printBlockParms[1] = (uint64_t)dst;
    printBlockParms[2] = (uint64_t)blockNum;

    asm("CLI");
    PROC_create_kthread(printBlock, printBlockParms);
    asm("STI");

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
        printk_err("[initBlockDevice]: passed in the wrong number of parameters, expected 4 and received %d\n", arrLen - 1);
        kexit();
    }
    uint16_t base = paramArr[1];
    uint16_t master = paramArr[2];
    uint8_t slave = paramArr[3];
    uint8_t irq = paramArr[4];

    ata = ata_probe(base, master, slave, irq);
    kexit();
}

/* void readBlock0(){ */
/*     char dest[512], dest2[512]; */
/*     memset(dest, 0, 512); */
/*     ata->read_block(ata, 2048, dest); */

/*     ExtendedFatBootRecord *efbr = (ExtendedFatBootRecord*)dest; */

/*     readBPB(&(efbr->bpb)); */

/*     unsigned int root_dir_sector = get_root_directory_sector((ExtendedFatBootRecord*)dest); */
/*     printk("root_dir_sector = %u\n", root_dir_sector); */

/*     uint32_t fat_size = efbr->sectors_per_fat; */
/*     uint64_t root_dir_sectors = 0; //0 on FAT32 */
/*     uint64_t first_data_sector = efbr->bpb.num_reserved_sectors + (efbr->bpb.num_fats * fat_size) + root_dir_sectors; */
/*     //uint64_t first_fat_sector = efbr->bpb.num_reserved_sectors; */
/*     printk("first data sector = %lu\n", first_data_sector + 2048); */

/*     // read in the root dir block */
/*     ata->read_block(ata,  first_data_sector + 2048, dest2); */
/*     // parse entries in the root dir */
/*     printk("--------- ROOT DIRECTORY ---------\n"); */
/*     uint8_t *entry = (uint8_t*)dest2; */
/*     read_directory(entry); */
/*     kexit(); */
/* } */

int init_pci_devices(){
    int num_devices_initd = 0;
    PCIDevice *cur = pci_devices;

    while(cur){
	//printk("dev vendor = 0x%x\n", cur->vendor_id);
	switch(cur->vendor_id){
	case 0x10ec:
	    //printk("found a realtek device\n");
	    switch(cur->device_id){
	    case 0x8139:
	        if(!init_rt8139(cur)){
		    num_devices_initd += 1;
		}
		break;
	    }
	    
	    break;
	}
	cur = cur->next_device;
    }

    return num_devices_initd;
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

/* void configure_kmain_from_env(){ */
/*     if(getenv("KMAIN_NODEBUG")){ */
/* 	no_debug = 1; */
/*     } */

/*     char *print_level = getenv("KMAIN_PRINTLEVEL"); */
/*     if(print_level){ */
/* 	if(strcmp(print_level, "INFO") == 0){ */
/* 	    printk_conf = info; */
/* 	} */
/* 	else if(strcmp(print_level, "WARN") == 0){ */
/* 	    printk_conf = warn; */
/* 	} */
/* 	else if(strcmp(print_level, "ERR") == 0){ */
/* 	    printk_conf = err; */
/* 	} */
/* 	else if(strcmp(print_level, "SILENT") == 0){ */
/* 	    printk_conf = silent; */
/* 	} */
/*     } */
/* } */

int kmain(void *multiboot_point, unsigned int multitest){
  asm("cli");

  //set up and test the VGA
  VGA_clear();
  // vgaDispCharTest();
  // VGA_display_str("string print test\n");

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
  IRQ_clear_mask(SLAVE_PIC_IRQ);
  IRQ_clear_mask(PRIMARY_ATA_CHANNEL_IRQ);
  // IRQ_clear_mask(SECONDARY_ATA_CHANNEL_IRQ);

  init_kbd_state();

  //turn on interrupts
  asm("sti");

  //initialize serial output
  SER_init();
  printk("----SERIAL DEBUGGING BEGIN----\n");
  int v = 1;

  char *y = (char*)&v;

  printk_info("%s\n",*y ? "Endianness: Little Endian" : "Endianness: Big Endian");
  
  //initialize the memory info datastructure
  init_usable_segment_struct();

  //take the multiboot_point from entering long mode, and figure out where tags BEGIN
  TagStructureInfo *tagStructureInfo = (TagStructureInfo*)multiboot_point;
  printk("tagStructInfo: total_size=%u reserved=%u\n", tagStructureInfo->totalBytes, tagStructureInfo->reserved);
  GenericTagHeader *curTag = (GenericTagHeader*)((uint64_t)tagStructureInfo + 8);

  //iterate over and parse all multiboot tags
  while(curTag){
    printTagInfo(curTag);
    potentiallyUseTag(curTag);
      curTag = getNextTag(curTag);
  }

  store_control_registers();
  uint64_t ****new_page_table = init_page_table();
  load_page_table((uint64_t)new_page_table);

  init_kheap();
  ata = ata_probe(0x1F0, 0x40, 0, 0x14);

  printk_info("Searching for connected PCI devices:\n");
  int num_pci_devs = pci_probe(&pci_devices);
  printk_info("Found %d pci devices\n", num_pci_devs);

  if(num_pci_devs){
      init_pci_devices();
  }
  
  int enabled = 0;
  while(!enabled);
  
  /* uint64_t initFAT32Params[2] = {2, (uint64_t)ata}; */
  /* PROC_create_kthread(initFAT32, initFAT32Params); */
  
  while(1){
      PROC_run();
      asm("hlt");
  }

  return 0;
}
