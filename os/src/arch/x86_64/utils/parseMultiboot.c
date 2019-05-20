#include "parseMultiboot.h"
#include "drivers/memory/memoryManager.h"
#include "utils/printk.h"

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
    int size = curTag->size; //, type = curTag->type;
    while(size % 8 != 0) size += 1;

    //if unrecognized type, assume garbage and no next tag
    if(curTag->type < 0 || curTag->type > 21){
      return 0;
    }

    //get the earliest possible address of the next tag (no padding)
    nextTagAddr = ((uint64_t)curTag) + size;
    nextTagHead = (GenericTagHeader*)nextTagAddr;
    while(nextTagHead->type == MBTAG_PADDING_TAG){
	//must have gone past the ending, returning null
	if(nextTagAddr > tagEnding){
	    printk_err("tried to get a tag after the ending\n");
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

    case MBTAG_BOOT_COMMAND_LINE:
	printk_info("[BOOT_COMMAND_LINE-%d]\n", MBTAG_BOOT_COMMAND_LINE);
	char *cmdLineStr = (char *)tag + 8 + internalPadding + tag->size;
	printk("   boot commandLine: %s\n", cmdLineStr);
	break;

    case MBTAG_BOOTLOADER_NAME:
	printk_info("[BOOT_LOADER_NAME-%d]\n", MBTAG_BOOTLOADER_NAME);
	BootLoaderName *bootloader_name = (BootLoaderName*)tag;

	printk("   bootloader name: %s\n", &(bootloader_name->bootLoaderName));
	break;

    case MBTAG_BOOTLOADER_MODULE:
	printk_info("[BOOT_LOADER_MODULE-%d]\n", MBTAG_BOOTLOADER_MODULE);
	BootLoaderModule *mod = (BootLoaderModule*)tag;
	
	printk("    mod_start = %d\nmod_end = %d\nmod_name = %s\n", mod->mod_start, mod->mod_end, mod->module_name);
	break;

    case MBTAG_BASIC_MEMORY_INFO:
	printk_info("[BASIC_MEMORY_INFO-%d]\n", MBTAG_BASIC_MEMORY_INFO);
	break;

    case MBTAG_BIOS_BOOT_DEVICE:
	printk_info("[BIOS_BOOT_DEVICE_TAG_TYPE-%d]\n", MBTAG_BIOS_BOOT_DEVICE);
	break;

    case MBTAG_MEMORY_MAP:
	printk_info("[MEMORY_MAP-%d]\n", MBTAG_MEMORY_MAP);
	MemoryMap *mapTag = (MemoryMap*) tag;

	uint64_t endOfTag = (uint64_t)mapTag + mapTag->size;

	printk("    MapInfo: entrySize=%d, entryVersion=%d\n", mapTag->memoryInfoEntrySize, mapTag->version);
	MemoryMapEntry * entry = &(mapTag->map);
	while((uint64_t)entry < endOfTag){
	    printk("        startingAddr=%lx, lengthInBytes %lx, type=%d, reserved=%d\n", entry->startingAddr, entry->lengthInBytes, entry->type, entry->reserved);
	    entry = (MemoryMapEntry*)(((uint64_t)entry) + mapTag->memoryInfoEntrySize);
	}
	break;

    case MBTAG_VBE_INFO:
	printk_info("[VBE_INFO-%d]\n", MBTAG_VBE_INFO);
	break;

    case MBTAG_FRAME_BUFFER_INFO:
	printk_info("[FRAME_BUFFER_INFO-%d]\n", MBTAG_FRAME_BUFFER_INFO);
	break;

    case MBTAG_ELF_SYMBOLS:
	printk_info("[ELF_SYMBOLS-%d]\n", MBTAG_ELF_SYMBOLS);
	ElfSymbols *elfs = (ElfSymbols*) tag;
	printk("    elfs summary: size=%d, numSectionHeaderEntries=%d, sizeOfSectionHeaderEntry=%d, sectionIndexContainingStrTable=%d\n", elfs->size, elfs->numSectionHeaderEntries, elfs->sizeOfSectionHeaderEntry, elfs->sectionIndexContainingStrTable);
	int i;
	ElfSectionHeader *elfArrPointer = &(elfs->headerArr);
	for(i = 0; i < elfs->numSectionHeaderEntries; i++){
	    printk("        Elf info: type=%d, address=%lx, size=%lx, offset=%lx\n", elfArrPointer->typeOfSection, elfArrPointer->segmentAddr, elfArrPointer->sizeOfSegment, elfArrPointer->segmentOffsetOnDisk);
	    elfArrPointer++;
	}
	break;

    case MBTAG_APM_TABLE:
	printk_info("[APM_TABLE-%d]\n", MBTAG_APM_TABLE);
	break;

    case MBTAG_EFI32_SYSTEM_TABLE:
	printk_info("[EFI32_SYSTEM_TABLE-%d]\n", MBTAG_EFI32_SYSTEM_TABLE);
	break;

    case MBTAG_EFI64_SYSTEM_TABLE:
	printk_info("[EFI64_SYSTEM_TABLE-%d]\n", MBTAG_EFI64_SYSTEM_TABLE);
	break;

    case MBTAG_SMBIOS_TABLES:
	printk_info("[MBTAG_SMBIOS_TABLES-%d]\n", MBTAG_SMBIOS_TABLES);
	break;

    case MBTAG_ACPI_OLD_RSDP:
	printk_info("[ACPI_OLD_RSDP-%d]\n", MBTAG_ACPI_OLD_RSDP);
	break;

    case MBTAG_ACPI_NEW_RSDP:
	printk_info("[ACPI_NEW_RSDP-%d]\n", MBTAG_ACPI_NEW_RSDP);
	break;

    case MBTAG_NETWORKING_INFO:
	printk_info("[NETWORKING_INFO-%d]\n", MBTAG_NETWORKING_INFO);	
	break;

    case MBTAG_EFI_MEMMAP:
	printk_info("[EFI_MEMMAP-%d]\n", MBTAG_EFI_MEMMAP);	
	break;

    case MBTAG_EFI_BOOT_NOT_TERM:
	printk_info("[EFI_BOOT_NOT_TERM-%d\n", MBTAG_EFI_BOOT_NOT_TERM);
	break;

    case MBTAG_EFI32_IMAGE_HANDLE:
	printk_info("[EFI32_IMAGE_HANDLE-%d\n", MBTAG_EFI32_IMAGE_HANDLE);
	break;

    case MBTAG_EFI64_IMAGE_HANDLE:
	printk_info("[EFI64_IMAGE_HANDLE-%d\n", MBTAG_EFI64_IMAGE_HANDLE);
	break;

    case MBTAG_LOAD_BASE_PHYS_ADDR:
	printk_info("[LOAD_BASE_PHYS_ADDR-%d\n", MBTAG_LOAD_BASE_PHYS_ADDR);

	LoadBasePhysAddr *lbpa = (LoadBasePhysAddr*) tag;
	printk("    Load Base Physical Addr: 0x%x\n", lbpa);
	break;

    default:
	printk_info("[ERR]: type (%d) not recognized\n", tag->type);
	break;
    }
}

void potentiallyUseTag(GenericTagHeader *tag){
    if(tag->type == MBTAG_MEMORY_MAP){
	printk_info("encountered memory_map tag\n");
	MemoryMap *mapTag = (MemoryMap*) tag;

	uint64_t endOfTag = (uint64_t)mapTag + mapTag->size;

	printk("MapInfo: entrySize=%d, entryVersion=%d\n", mapTag->memoryInfoEntrySize, mapTag->version);
	MemoryMapEntry * entry = &(mapTag->map);
	while((uint64_t)entry < endOfTag){
	    if(entry->type == 1){
		add_segment(entry->startingAddr, entry->startingAddr + entry->lengthInBytes, entry->lengthInBytes);
		printk("segment added\n");
	    }
	    update_end_of_memory(entry->startingAddr + entry->lengthInBytes);
	    entry = (MemoryMapEntry*)(((uint64_t)entry) + mapTag->memoryInfoEntrySize);
	}
    }
    else if(tag->type == MBTAG_ELF_SYMBOLS){
	printk_info("encountered elf_symbols tag\n");
	ElfSymbols *elfs = (ElfSymbols*) tag;
	printk_info("elfs summary: size=%d, numSectionHeaderEntries=%d, sizeOfSectionHeaderEntry=%d, sectionIndexContainingStrTable=%d\n", elfs->size, elfs->numSectionHeaderEntries, elfs->sizeOfSectionHeaderEntry, elfs->sectionIndexContainingStrTable);
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
	add_blocked_segment(earliest_elf_address, last_elf_address);
    }
}
