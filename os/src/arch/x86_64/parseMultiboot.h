#ifndef PARSE_MULTIBOOT
#define PARSE_MULTIBOOT

#include "printk.h"
#include <stdint-gcc.h>

#define PADDING_TAG 0
#define BOOT_COMMAND_LINE 1
#define BOOT_LOADER_NAME 2
#define BASIC_MEMORY_INFO_TAG_TYPE 4
#define BIOS_BOOT_DEVICE_TAG_TYPE 5
#define MEMORY_MAP 6
#define VBE_INFO 7
#define FRAME_BUFFER_INFO 8
#define ELF_SYMBOLS 9
#define APM_TABLE 10

typedef struct {
    uint32_t totalBytes;
    uint32_t reserved;
}__attribute__((packed)) TagStructureInfo;

typedef struct {
    uint32_t type;
    uint32_t size;
}__attribute__((packed)) GenericTagHeader;

typedef struct {
    uint32_t type;
    uint32_t size;

    //number of lower memory kilobytes
    //starts at address 0
    uint32_t mem_lower;

    //number of upper memory kilobytes
    //starts at addres 1Mb
    uint32_t mem_upper;
}__attribute__((packed)) BasicMemoryInfoTag;

typedef struct {
    uint32_t type;
    uint32_t size;

    uint32_t biosBootDevice;
    uint32_t biosBootPartition;
    uint32_t biosBootApplication;
}__attribute__((packed)) BIOSBootDeviceTag;

typedef struct {
    uint32_t type;
    uint32_t size;
    
    //variable size c string, null terminated
    char cmdLineCStr;
}__attribute__((packed)) BootCommandLineTag;

typedef struct {
    uint32_t type;
    uint32_t size;

    //variable size c string, null terminated
    char *bootLoaderName;
}__attribute__((packed)) BootLoaderName;

//only pay attention to type 1 entries (only ones that are free for the OS to use)
typedef struct {
    uint64_t startingAddr;
    uint64_t lengthInBytes;
    uint32_t type;
    uint32_t reserved;
}__attribute__((packed)) MemoryMapEntry;

typedef struct {
    uint32_t type;
    uint32_t size;

    uint32_t memoryInfoEntrySize;
    uint32_t version;
    MemoryMapEntry *map;
}__attribute__((packed)) MemoryMap;

typedef struct {
    uint32_t sectionNameAsIndex;
    uint32_t typeOfSection;

    uint64_t flags;
    uint64_t segmentAddr;
    uint64_t segmentOffsetOnDisk;
    uint64_t sizeOfSegment;

    uint32_t tableIndexLink;
    uint32_t extraInfo;

    uint64_t addressAlignment; //powers of 2
    uint64_t fixedEntrySizeIfFixed;
}__attribute__((packed)) ElfSectionHeader;

typedef struct {
    uint32_t type;
    uint32_t size;

    uint32_t numSectionHeaderEntries;
    uint32_t sizeOfSectionHeaderEntry;
    uint32_t sectionIndexContainingStrTable;

    ElfSectionHeader *headerArr;
}__attribute__((packed)) ElfSymbols;

//functions
uint32_t getTotalTagSize(TagStructureInfo*);
GenericTagHeader *getNextTag(GenericTagHeader*);
void printTagInfo(GenericTagHeader*);

#endif
