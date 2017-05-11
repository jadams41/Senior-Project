#include "parseMultiboot.h"
#include "memoryManager.h"

static uint64_t tagBeginning;
static uint64_t tagEnding;

uint32_t getTotalBytes(){
    return ((TagStructureInfo*)tagBeginning)->totalBytes;
}

/*
 * @param tagHeaderPointer the pointer loaded by GRUB in %ebx when entering protected mode
 * @return total size of all tags (including terminating tag) in bytes
 */
uint32_t getTotalTagSize(TagStructureInfo *info){
    tagBeginning = (uint64_t) info;
    tagEnding = tagBeginning + info->totalBytes;
    return info->totalBytes;
}

GenericTagHeader *getNextTag(GenericTagHeader *curTag){
    uint64_t nextTagAddr;
    GenericTagHeader *nextTagHead;

    //8-byte align the size
    int size = curTag->size, type = curTag->type;
    while(size % 8 != 0) size += 1;

    //if unrecognized type, assume garbage and no next tag
    if(type < 0 || type == 3 || type > 10){
        return 0;
    }

    //get the earliest possible address of the next tag (no padding)
    nextTagAddr = ((uint64_t)curTag) + size;
    nextTagHead = (GenericTagHeader*)nextTagAddr;
    while(nextTagHead->type == PADDING_TAG){
        //must have gone past the ending, returning null
        if(nextTagAddr > tagEnding){
            printk("[ERR]: tried to get a tag after the ending");
            return 0;
        }
        size = nextTagHead->size;
        while(size % 8 != 0) size += 1;
        nextTagAddr += size;
        nextTagHead = (GenericTagHeader*)nextTagAddr;
    }
    return nextTagHead;
}

int getNumPaddingBytes(GenericTagHeader *tag){
    int realSize = tag->size;
    int alignedSize = realSize;
    while(alignedSize % 8) alignedSize += 1;

    return alignedSize - realSize;
}

void printTagInfo(GenericTagHeader *tag){
    int internalPadding = getNumPaddingBytes(tag);
    switch(tag->type){
        case BOOT_COMMAND_LINE:
            printk("[BOOT_COMMAND_LINE-%d]\n", BOOT_COMMAND_LINE);
            char *cmdLineStr = (char *)tag + 8 + internalPadding + tag->size;
            printk("   boot commandLine: %s\n", cmdLineStr);
            break;
        case BOOT_LOADER_NAME:
            printk("[BOOT_LOADER_NAME-%d]\n", BOOT_LOADER_NAME);
            char *bootLoaderName = (char *)tag + 8 + internalPadding + tag->size;
            printk("   bootloader name: %s\n", bootLoaderName);
            break;
        case BASIC_MEMORY_INFO_TAG_TYPE:
            printk("[BASIC_MEMORY_INFO-%d]\n", BASIC_MEMORY_INFO_TAG_TYPE);
            break;
        case BIOS_BOOT_DEVICE_TAG_TYPE:
            printk("[BIOS_BOOT_DEVICE_TAG_TYPE-%d]\n", BIOS_BOOT_DEVICE_TAG_TYPE);
            break;
        case MEMORY_MAP:
            printk("[MEMORY_MAP-%d]\n", MEMORY_MAP);
            MemoryMap *mapTag = (MemoryMap*) tag;

            uint64_t endOfTag = (uint64_t)mapTag + mapTag->size;

            printk("MapInfo: entrySize=%d, entryVersion=%d\n", mapTag->memoryInfoEntrySize, mapTag->version);
            MemoryMapEntry * entry = &(mapTag->map);
            while((uint64_t)entry < endOfTag){
                printk("startingAddr=%lx, lengthInBytes %lx, type=%d, reserved=%d\n", entry->startingAddr, entry->lengthInBytes, entry->type, entry->reserved);
                entry = (MemoryMapEntry*)(((uint64_t)entry) + mapTag->memoryInfoEntrySize);
            }
            break;
        case VBE_INFO:
            printk("[VBE_INFO-%d]\n", VBE_INFO);
            break;
        case FRAME_BUFFER_INFO:
            printk("[FRAME_BUFFER_INFO-%d]\n", FRAME_BUFFER_INFO);
            break;
        case ELF_SYMBOLS:
            printk("[ELF_SYMBOLS-%d]\n", ELF_SYMBOLS);
            ElfSymbols *elfs = (ElfSymbols*) tag;
            printk("elfs info: size=%d, numSectionHeaderEntries=%d, sizeOfSectionHeaderEntry=%d, sectionIndexContainingStrTable=%d\n", elfs->size, elfs->numSectionHeaderEntries, elfs->sizeOfSectionHeaderEntry, elfs->sectionIndexContainingStrTable);
            int i;
            ElfSectionHeader *elfArrPointer = &(elfs->headerArr);
            for(i = 0; i < elfs->numSectionHeaderEntries; i++){
                printk("Elf info: type=%d, address=%lx, size=%lx, offset=%lx\n", elfArrPointer->typeOfSection, elfArrPointer->segmentAddr, elfArrPointer->sizeOfSegment, elfArrPointer->segmentOffsetOnDisk);
                elfArrPointer++;
            }
            break;
        case APM_TABLE:
            printk("[APM_TABLE-%d]\n", APM_TABLE);
            break;
        default:
            printk("[ERR]: type (%d) not recognized\n", tag->type);
            break;
    }
}

void potentiallyUseTag(GenericTagHeader *tag, memory_info* memInfo){
    if(tag->type == MEMORY_MAP){
        MemoryMap *mapTag = (MemoryMap*) tag;

        uint64_t endOfTag = (uint64_t)mapTag + mapTag->size;

        printk("MapInfo: entrySize=%d, entryVersion=%d\n", mapTag->memoryInfoEntrySize, mapTag->version);
        MemoryMapEntry * entry = &(mapTag->map);
        while((uint64_t)entry < endOfTag){
            if(entry->type == 1){
                add_segment(memInfo, entry->startingAddr, entry->startingAddr + entry->lengthInBytes, entry->lengthInBytes);
                printk("segment added\n");
            }
            entry = (MemoryMapEntry*)(((uint64_t)entry) + mapTag->memoryInfoEntrySize);
        }
    }
    else if(tag->type == ELF_SYMBOLS){
        ElfSymbols *elfs = (ElfSymbols*) tag;
        printk("elfs info: size=%d, numSectionHeaderEntries=%d, sizeOfSectionHeaderEntry=%d, sectionIndexContainingStrTable=%d\n", elfs->size, elfs->numSectionHeaderEntries, elfs->sizeOfSectionHeaderEntry, elfs->sectionIndexContainingStrTable);
        int i;
        uint64_t earliest_elf_address = -1, last_elf_address = 0;
        ElfSectionHeader *elfArrPointer = &(elfs->headerArr);
        for(i = 0; i < elfs->numSectionHeaderEntries; i++){
            uint64_t beginning = elfArrPointer->segmentAddr;
            uint64_t size = elfArrPointer->sizeOfSegment;
            uint64_t end = beginning + size;
            if(size != 0){
                if(beginning < earliest_elf_address)
                    earliest_elf_address = beginning;
                if(end > last_elf_address)
                    last_elf_address = end;
            }
            elfArrPointer++;
        }
        printk("going to block chunk of memory: (%lx, %lx)\n", earliest_elf_address, last_elf_address);
        add_blocked_segment(memInfo, earliest_elf_address, last_elf_address);
    }
}
