/* Multiboot2 spec reference: https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html */

#ifndef PARSE_MULTIBOOT
#define PARSE_MULTIBOOT

#include "drivers/memory/memoryManager.h"
#include <stdint-gcc.h>

/* Multiboot2 tag types */
#define MBTAG_PADDING_TAG 0
#define MBTAG_BOOT_COMMAND_LINE 1
#define MBTAG_BOOTLOADER_NAME 2
#define MBTAG_BOOTLOADER_MODULE 3
#define MBTAG_BASIC_MEMORY_INFO 4
#define MBTAG_BIOS_BOOT_DEVICE 5
#define MBTAG_MEMORY_MAP 6
#define MBTAG_VBE_INFO 7
#define MBTAG_FRAME_BUFFER_INFO 8
#define MBTAG_ELF_SYMBOLS 9
#define MBTAG_APM_TABLE 10
#define MBTAG_EFI32_SYSTEM_TABLE 11
#define MBTAG_EFI64_SYSTEM_TABLE 12
#define MBTAG_SMBIOS_TABLES 13
#define MBTAG_ACPI_OLD_RSDP 14
#define MBTAG_ACPI_NEW_RSDP 15
#define MBTAG_NETWORKING_INFO 16
#define MBTAG_EFI_MEMMAP 17
#define MBTAG_EFI_BOOT_NOT_TERM 18
#define MBTAG_EFI32_IMAGE_HANDLE 19
#define MBTAG_EFI64_IMAGE_HANDLE 20
#define MBTAG_LOAD_BASE_PHYS_ADDR 21

/* Framebuffer Info framebuffer_types */
#define FBTYPE_INDEXED_COLOR 0
#define FBTYPE_DIRECT_RGB_COLOR 1
#define FBTYPE_EGA_TEXT 2

typedef struct {
    uint32_t totalBytes;
    uint32_t reserved;
}__attribute__((packed)) TagStructureInfo;

typedef struct {
    uint32_t type;
    uint32_t size;
}__attribute__((packed)) GenericTagHeader;

/****** BOOT COMMAND LINE TAG (type = 1) ******/
/* +-------------------+
   | type = 1          | u32
   | size              | u32
   | string            | u8[n]
   +-------------------+ */
typedef struct {
    uint32_t type; /* = 1 */
    uint32_t size;

    //variable size c string, null terminated
    char cmdLineCStr;
}__attribute__((packed)) BootCommandLineTag;

/****** BOOT LOADER NAME TAG (type = 2) ******/
/* +-------------------+
   | type = 2          | u32
   | size              | u32
   | string            | u8[n]
   +-------------------+ */
typedef struct {
    uint32_t type; /* = 2 */
    uint32_t size;

    //variable size c string, null terminated
    char bootLoaderName;
}__attribute__((packed)) BootLoaderName;

/****** BOOTLOADER MODULE TAG (type = 3) ******/
/* +-------------------+
   | type = 3          | u32
   | size              | u32
   | mod_start         | u32
   | mod_end           | u32
   | string            | u8[n]
   +-------------------+ */
typedef struct {
    uint32_t type; /* = 3 */
    uint32_t size;

    //start and end physical addresses of the boot module itself
    uint32_t mod_start;
    uint32_t mod_end;

    //string of uknown length
    char module_name;
}__attribute__((packed)) BootLoaderModule;

/****** BASIC MEMORY INFO TAG (type = 4) ******/
/* +-------------------+
   | type = 4          | u32
   | size = 16         | u32
   | mem_lower         | u32
   | mem_upper         | u32
   +-------------------+ */
typedef struct {
    uint32_t type; /* = 4 */
    uint32_t size;

    //number of lower memory kilobytes
    //starts at address 0
    uint32_t mem_lower;

    //number of upper memory kilobytes
    //starts at addres 1Mb
    uint32_t mem_upper;
}__attribute__((packed)) BasicMemoryInfoTag;

/****** BIOS BOOT DEVICE TAG (type = 5) ******/
/* +-------------------+
   | type = 5          | u32
   | size = 20         | u32
   | biosdev           | u32
   | partition         | u32
   | sub_parition      | u32
   +-------------------+ */
typedef struct {
    uint32_t type; /* = 5 */
    uint32_t size;

    uint32_t biosBootDevice;
    uint32_t biosBootPartition;
    uint32_t biosBootApplication;
}__attribute__((packed)) BIOSBootDeviceTag;

/****** MEMORY INFO TAG (type = 6) ******/

//only pay attention to type 1 entries (only ones that are free for the OS to use)
typedef struct {
    uint64_t startingAddr;
    uint64_t lengthInBytes;
    uint32_t type;
    uint32_t reserved;
}__attribute__((packed)) MemoryMapEntry;

/* ~~~~~~~~+-------------------+
   u32     | type = 6          |
   u32     | size              |
   u32     | entry_size        | "size of each entry. guaranteed to be a multiple of 8"
   u32     | entry_version     | "should be set at '0'. May be changed in future versions."
   varies  | entries           | Array of Memory Map Entries.
   ~~~~~~~~+-------------------+ 
*/
typedef struct {
    uint32_t type; /* = 6 */
    uint32_t size;

    uint32_t memoryInfoEntrySize;
    uint32_t version;
    MemoryMapEntry map;
}__attribute__((packed)) MemoryMap;

/****** VBE INFO TAG (type = 7) ******/ 

/* Note: VBE standard defines a set of functions related to the operatioan of the video card, and specifies how they can be invoked.
   ~~~~~~~~+-------------------+
   u32     | type = 7          |
   u32     | size = 784        |
   u16     | vbe_mode          | indicates current video mode in the format specified in vbe 3.0.
   u16     | vbe_interface_seg | references table of a protected mode interface defined in vbe 2.0+
   u16     | vbe_interface_off | references table of a protected mode interface defined in vbe 2.0+
   u16     | vbe_interface_len | references table of a protected mode interface defined in vbe 2.0+
   u8[512] | vbe_control_info  | VBE control information returned by VBE function: 00h
   u8[256] | vbe_mode_info     | VBE control information returned by VBE function: 01h
   ~~~~~~~~+-------------------+ */
typedef struct {
    uint32_t type;
    uint32_t size;

    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint8_t vbe_control_info[512];
    uint8_t vbe_mode_info[256];
}__attribute__((packed)) VBEInfo;

/****** Framebuffer Info Tag (type = 8) ******/

/* Note: A framebuffer is a portion of RAM containing a bitmap that drives a video display.
   ~~~~~~~~+--------------------+
   u32     | type = 8           |
   u32     | size               |
   u64     | framebuffer_addr   | physical address of the framebuffer.
   u32     | framebuffer_pitch  | pitch of framebuffer (number of bytes that are on each row of the screen).
   u32     | framebuffer_width  | framebuffer width (in pixels).
   u32     | framebuffer_height | framebuffer height (in pixels).
   u8      | framebuffer_bpp    | number of bits per pixel.
   u8      | framebuffer_type   | if 0 -> indexed color. if 1 -> direct RGB color. 2 -> EGA text.
   u8      | reserved           | always 0
   varies  | color_info         | different structure according to framebuffer type
   ~~~~~~~~+--------------------+ */
typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved;
}__attribute__((packed)) FramebufferInfo;

typedef struct {
    uint8_t red_value;
    uint8_t green_value;
    uint8_t blue_value;
}__attribute__((packed)) ColorDescriptor;

/* extends FramebufferInfo (framebuffer_type = 0) */
typedef struct {
    FramebufferInfo info;

    //begin index color color_info
    uint32_t frame_buffer_palette_num_colors;
    //framebuffer_palette: ColorDescriptor[]
    ColorDescriptor framebuffer_palette; //first color in the array
    
}__attribute__((packed)) IndexedColorFramebufferInfo;

/* extends FramebufferInfo (framebuffer_type = 1) */
typedef struct {
    FramebufferInfo info;
    
    //begin direct rgb color info
    uint8_t framebuffer_red_field_position;
    uint8_t framebuffer_red_mask_size;
    uint8_t framebuffer_green_field_position;
    uint8_t framebuffer_green_mask_size;
    uint8_t framebuffer_blue_field_position;
    uint8_t framebuffer_blue_mask_size;
}__attribute__((packed)) DirectRGBFrameBufferInfo;

/* extends FramebufferInfo (framebuffer_type = 2) */
typedef struct {
    //framebuffer_width is expressed in characters instead of pixels
    //framebuffer_height is expressed in characters instead of pixels
    //framebuffer_bpp = 16 (16 bits per character)
    //framebuffer_pitch is expressed in bytes per test line.
    //all other values are meaaningless
    FramebufferInfo info;
    // no colorinfo field
}__attribute__((packed)) EGATestFrameBufferInfo;

/****** ELF SYMBOLS TAG (type = 9) ******/
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
    uint32_t type; /* = 9 */
    uint32_t size;

    uint32_t numSectionHeaderEntries;
    uint32_t sizeOfSectionHeaderEntry;
    uint32_t sectionIndexContainingStrTable;

    ElfSectionHeader headerArr;
}__attribute__((packed)) ElfSymbols;

/****** IMAGE LOAD BASE PHYSICAL ADDRESS TAG (type = 21) ******/
typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t load_base_addr;
}__attribute__((packed)) LoadBasePhysAddr;


//functions
uint32_t getTotalTagSize(TagStructureInfo*);
GenericTagHeader *getNextTag(GenericTagHeader*);
void printTagInfo(GenericTagHeader*);
void potentiallyUseTag(GenericTagHeader *tag);

#endif
