#include <stdint-gcc.h>
#include "drivers/block/blockDeviceDriver.h"
#include "drivers/memory/memoryManager.h"
#include "types/string.h"
#include "utils/utils.h"
#include "utils/printk.h"
#include "types/process.h"

static ProcessQueue blockDevicePending; //processes waiting on response from disk

static int ata_48_read_block(struct BlockDev *dev, uint64_t blk_num, void *dst){
    uint8_t sectorCountHigh = (1 & 0xff00) >> 8;
    uint8_t sectorCountLow = 1 & 0xff;
    uint8_t lba1 = blk_num & 0xff;
    uint8_t lba2 = (blk_num & 0xff00) >> 8;
    uint8_t lba3 = (blk_num & 0xff0000) >> 16;
    uint8_t lba4 = (blk_num & 0xff000000) >> 24;
    uint8_t lba5 = (blk_num & 0xff00000000) >> 32;
    uint8_t lba6 = (blk_num & 0xff0000000000) >> 40;

    outb(0x1F6, 0x40);
    outb (0x1F2, sectorCountHigh);
    outb (0x1F3, lba4);
    outb (0x1F4, lba5);
    outb (0x1F5, lba6);
    outb (0x1F2, sectorCountLow);
    outb (0x1F3, lba1);
    outb (0x1F4, lba2);
    outb (0x1F5, lba3);
    outb(0x1F7, 0x24);

    //block here
    asm("CLI");
    PROC_block_on(&blockDevicePending, 0);
    asm("CLI");

    char *buffer = (char*)dst;

    int i, count = 512;
    unsigned short s;
    for (i = 0; i < count; i += 2) {
        s = inw(0x1f0);
        buffer[i] = s & 0xff;
        buffer[i+1] = s >> 8;
    }

    return 1;
}

int queueReadOperation(struct BlockDev *dev, uint64_t blk_num, void *dst){
    asm("CLI");
    struct ATABlockDev *ata = (struct ATABlockDev*)dev;

    //keep blocking while the BlockDevice is locked (another operation in progress)
    while(ata->dev.locked){
        asm("CLI");
        PROC_block_on(&(ata->operationQueue), 0);
    }
    asm("CLI");

    //lock the process ourselves and let it rip
    ata->dev.locked = 1;
    int numRead = ata_48_read_block(dev, blk_num, dst);

    //operation is now done, so release lock
    ata->dev.locked = 0;

    //unblock the next operation (there might not be one)
    PROC_unblock_head(&(ata->operationQueue));

    return numRead;
}


//return 1 if a process was unblocked, otherwise 0
int handleAndDequeueBlockDevicePending(){
    uint8_t status;
    ATAStatusByte *statusByte = (ATAStatusByte*)&status;


    int numTimesToRead = 5;
    while(numTimesToRead-- > 0){
        status = inb(PRIMARY_ATA_CMD_OR_STATUS);
    }

    if(statusByte->err){
        printk_err("error bit was set in ata status, don't know how to handle other than death\n");
        asm("hlt");
    }

    if(blockDevicePending.head){
        PROC_unblock_head(&blockDevicePending);
        return 1;
    }
    else {
        printk_warn("No ATA request to unblock\n");
        return 0;
    }
}

// Initialize the ATA device
struct BlockDev *ata_probe(uint16_t base, uint16_t master, uint8_t slave, uint8_t irq) {
    blockDevicePending.head = 0;
    printk("*************** init ***************\n");
    //check the status byte
    uint8_t status;
    int numTimesToRead = 5;
    while(numTimesToRead-- > 0){
        status = inb(PRIMARY_ATA_CMD_OR_STATUS);
    }
    ATAStatusByte *statusByte = (ATAStatusByte*)&status;
    printk("ATA status information:\n  err: %d\n  drq: %d\n  srv: %d\n  df: %d\n  rdy: %d\n  bsy: %d\n",
        statusByte->err, statusByte->drq, statusByte->srv, statusByte->df, statusByte->rdy, statusByte->bsy
    );

    //IDENTIFY COMMAND PROTOCOL
    outb(PRIMARY_ATA_DRIVE_OR_HEAD, 0xA0); //selecting the master drive (0xA0) on drive select port
    outb(PRIMARY_ATA_SECTOR_COUNT, 0x0); //set the sectorcount to 0
    outb(PRIMARY_ATA_LBA_LOW, 0x0); //set the LBAlo to 0
    outb(PRIMARY_ATA_LBA_MID, 0x0); //set the LBAmid to 0
    outb(PRIMARY_ATA_LBA_HIGH, 0x0); //set the LBAhi to 0
    outb(PRIMARY_ATA_CMD_OR_STATUS, ATA_IDENTIFY);

    status = inb(PRIMARY_ATA_CMD_OR_STATUS);
    if(status == 0){
        printk_err("master drive apparently doesn't exist\n");
        kexit();
    }
    //poll the status port until device not busy
    while(statusByte->bsy){
        status = inb(PRIMARY_ATA_CMD_OR_STATUS);
    }
    //make sure that LBAmid and LBAhi are still 0
    status = inb(PRIMARY_ATA_LBA_MID);
    uint16_t status2 = inb(PRIMARY_ATA_LBA_HIGH);
    if(status == 0x14 && status2 == 0xEB){
        printk_info("device type is ATAPI\n");
        kexit();
    }
    else if(status == 0x3c && status2 == 0xc3){
        printk_info("device type is SATA\n");
        kexit();
    }

    //poll status until either err or drq is set
    status = inb(PRIMARY_ATA_CMD_OR_STATUS);
    while(1){
        if(statusByte->err){
            printk_err("there was an error with something in the ATA\n");
            kexit();
        }
        if(statusByte->drq){
            printk_info("ATA device is ready for PIO\n");
            break;
        }
        status = inb(PRIMARY_ATA_CMD_OR_STATUS);
    }

    //drq is set and ready to go
    uint16_t identifyDataValues[256];
    //read 256 16-bit values from DATA port and store them
    int i;
    for(i = 0; i < 256; i++){
        identifyDataValues[i] = inw(PRIMARY_ATA_DATA);
    }
    status = inb(PRIMARY_ATA_DATA);
    if(identifyDataValues[83] & (1 << 10)){
        printk_info("this device supports LBA48 mode\n");
    }
    uint64_t *numSectors = (uint64_t*)(identifyDataValues + 100);
    printk_info("the number of available sectors on the disk: 0x%lx\n", *numSectors);

    struct ATABlockDev *ata;
    ata = kmalloc(sizeof(*ata));
    memset(ata, 0, sizeof(*ata));

    ata->ata_base = base;
    ata->slave = slave;
    ata->ata_master = master;
    ata->irq = irq;
    ata->dev.locked = 0;

    ata->dev.read_block = &queueReadOperation;

    printk("*************** endinit ***************\n");

    return (struct BlockDev*)ata;
}
