#include "parseMultiboot.h"

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

    //get the earliest possible address of the next tag (no padding)
    nextTagAddr = ((uint64_t)curTag) + curTag->size;
    nextTagHead = (GenericTagHeader*)nextTagAddr;
    while(nextTagHead->type == PADDING_TAG){
        //must have gone past the ending, returning null
        if(nextTagAddr > tagEnding){
            printk("[ERR]: tried to get a tag after the ending");
            return 0;
        }

        nextTagAddr += nextTagHead->size;
        nextTagHead = (GenericTagHeader*)nextTagAddr;
    }
    return nextTagHead;
}

void printTagInfo(GenericTagHeader *tag){
    switch(tag->type){
        case BOOT_COMMAND_LINE:
            printk("[BOOT_COMMAND_LINE]\n");
            BootCommandLineTag *cmdLine = (BootCommandLineTag*)tag;
            char *cmdLineStr = &cmdLine->cmdLineCStr;
            printk("CMD: %s", cmdLineStr);
            break;
        case BOOT_LOADER_NAME:
            printk("[BOOT_LOADER_NAME]\n");
            break;
        case BASIC_MEMORY_INFO_TAG_TYPE:
            printk("[BASIC_MEMORY_INFO]\n");
            break;
        case BIOS_BOOT_DEVICE_TAG_TYPE:
            printk("[BIOS_BOOT_DEVICE_TAG_TYPE]\n");
            break;
        case MEMORY_MAP:
            printk("[MEMORY_MAP]\n");
            break;
        case VBE_INFO:
            printk("[VBE_INFO]\n");
            break;
        case FRAME_BUFFER_INFO:
            printk("[ELF_SYMBOLS]\n");
            break;
        case APM_TABLE:
            printk("[APM_TABLE]\n");
            break;
        default:
            printk("[ERR]: type (%d) not recognized\n", tag->type);
            break;
    }
}
