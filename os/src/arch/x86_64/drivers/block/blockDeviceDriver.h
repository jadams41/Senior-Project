#ifndef BLOCK_DEVICE_DRIVER
#define BLOCK_DEVICE_DRIVER

#include <stdint-gcc.h>
#include "types/process.h"

#define PRIMARY_ATA_DATA 0x1F0
#define PRIMARY_ATA_FEATURES 0x1F1
#define PRIMARY_ATA_ERROR_INFO 0x1F1
#define PRIMARY_ATA_SECTOR_COUNT 0x1F2
#define PRIMARY_ATA_SECTOR_NUM 0x1F3
#define PRIMARY_ATA_LBA_LOW 0x1F3
#define PRIMARY_ATA_CYL_LOW 0x1F4
#define PRIMARY_ATA_LBA_MID 0x1F4
#define PRIMARY_ATA_CYL_HIGH 0x1F5
#define PRIMARY_ATA_LBA_HIGH 0x1F5
#define PRIMARY_ATA_DRIVE_OR_HEAD 0x1F6
#define PRIMARY_ATA_CMD_OR_STATUS 0x1F7

#define ATA_IDENTIFY 0xEC

enum BlockDevType { MASS_STORAGE, PARTITION };
typedef enum request_type {READ, WRITE} request_type;

struct BlockDev {
    uint64_t tot_length;
    int (*read_block)(struct BlockDev *dev, uint64_t blk_num, void *dst);
    uint32_t blk_size;

    enum BlockDevType type;
    const char *name;

    uint8_t fs_type;
    struct BlockDev *next;

    int locked;
};

struct ATABlockDev {
    struct BlockDev dev;
    uint16_t ata_base, ata_master;
    uint8_t slave, irq;
    ProcessQueue operationQueue;
};

struct block_device_request {
    request_type type; //read, write
    uint64_t begnningBlock;
    uint64_t numBlocks;
    struct block_device_request *next;
};

typedef struct block_device_request block_device_request;
typedef struct BlockDev BlockDev;
typedef struct ATABlockDev ATABlockDev;

typedef struct {
    uint8_t err:1;
    uint8_t ignored:2;
    uint8_t drq:1;
    uint8_t srv:1;
    uint8_t df:1;
    uint8_t rdy:1;
    uint8_t bsy:1;
}__attribute__((packed)) ATAStatusByte ;

int queueReadOperation(struct BlockDev*,uint64_t,void*);
struct BlockDev *ata_probe(uint16_t, uint16_t, uint8_t, uint8_t);
int handleAndDequeueBlockDevicePending(void);
#endif
