#ifndef FAT_32
#define FAT_32

#include <stdint-gcc.h>
#include "drivers/block/blockDeviceDriver.h"

/*** STRUCTURE OF FAT32 PARTITION ***
     __________________________________
     |++++++++++++++++++++++++++++++++|
     |+       RESERVED SECTORS       +|
     |+++++++++++++++++++++++++++++++++
     |           Boot Sector          |
     |          FSInfo Sector         |
     | More Reserved Sctrs (optional) |
     |++++++++++++++++++++++++++++++++|
     |+ FILE ALLOCATION TABLE REGION +|
     |++++++++++++++++++++++++++++++++|
     |    File Allocation Table #1    |
     |        FAT #2 (optional)       |
     |++++++++++++++++++++++++++++++++|
     |+          DATA REGION         +|
     |++++++++++++++++++++++++++++++++|
     |           Data Sectors         |
     |________________________________|
 */

//FAT32 file attributes
#define FAT_ATTR_READ_ONLY 0x01
#define FAT_ATTR_HIDDEN 0x02
#define FAT_ATTR_SYSTEM 0x04
#define FAT_ATTR_VOLUME_ID 0x08
#define FAT_ATTR_DIRECTORY 0x10
#define FAT_ATTR_ARCHIVE 0x20

// long file file name entry
#define FAT_ATTR_LFN (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)

/* classes extending the VFS classes */
typedef struct {
    SuperBlock super;

    /* information taken from bpb and efbr */
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint32_t first_fat_sector;
    uint16_t sectors_per_fat;
    
    uint16_t fsinfo_sector_num;
    
    uint32_t total_sectors; //either total_sectors (16bit) or large_sector_count (32bit) if more than 2^16 sectors
    uint32_t num_hidden_sectors;


    /* information taken from FSInfo sector */
    uint8_t fsinfo_valid; //(0,1): 1 if all signatures are valid otherwise 0
    uint32_t last_known_free_clusters;
    uint32_t last_allocated_data_cluster;
    
    /* other FS information */
    uint64_t root_dir_sector;
    uint64_t first_data_sector;
    
} FAT32_SuperBlock;

typedef struct {
    Inode inode;

    // copy of FAT32 entry attributes
    uint8_t attributes;
} FAT32_Inode;

typedef struct {
    uint8_t inf_loop[3];
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t num_dir_entries; // should be 0 on FAT32 (since directory clusters are normal data clusters)
    uint8_t total_sectors[2];
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat; //fat12/16 only
    uint8_t sectors_per_track[2];
    uint8_t num_heads[2];
    uint8_t num_hidden_sectors[4];
    uint8_t large_sector_count[4]; //only set if total_sectors is 0
}__attribute__((packed)) BPB;

typedef struct {
    BPB bpb;
    uint32_t sectors_per_fat; //field ignored on FAT32
    uint16_t flags;
    uint8_t fat_version_major;
    uint8_t fat_version_minor;
    uint32_t root_dir_cluster_num;
    uint16_t fsinfo_sector_num; //todo figure out if there is any useful information here
    uint16_t backup_boot_sector_num;
    uint8_t reserved[12];
    uint8_t drive_num;
    uint8_t windowsNT_flags;
    uint8_t signature; //must be 0x28 or 0x29
    uint32_t vol_id_serial;
    uint8_t vol_label_str[11];
    uint8_t sys_id_str[8]; //always "FAT32 " (don't trust)
    uint8_t boot_code[420];
    uint16_t bootable_partition_sign; //0xAA55
}__attribute__((packed)) ExtendedFatBootRecord;


/* Note: information about this struct came from:
 * https://infogalactic.com/info/Design_of_the_FAT_file_system#FS_Information_Sector 
 */
typedef struct {
    uint8_t sig1; //should be 0x52
    uint8_t sig2; //should be 0x52
    uint8_t sig3; //should be 0x61
    uint8_t sig4; //should be 0x41

    uint8_t reserved[480];

    uint8_t sig5; //should be 0x72
    uint8_t sig6; //should be 0x72
    uint8_t sig7; //should be 0x41
    uint8_t sig8; //should be 0x61

    /* Last known number of free data clusters on the volume.
       0xFFFFFFFF if unknown.
       Note: Before using, sanity check that this is <= volume's count of clusters */
    uint32_t last_known_free_clusters;

    /* Number of the most recently knwon to be allocated data cluster.
       Should be set to 0xFFFFFFFF during format and updated by the operating system later on.
       If 0xFFFFFFFF, system should start at cluster 0x00000002. 
       Don't rely to be absolutely correct in all scenarios 
       Note: Before using, sanity check this value to be a valid cluster number on the volume */
    uint32_t last_allocated_data_cluster;

    uint8_t reserved2[12];

    uint8_t sig9;  //should be 0x00
    uint8_t sig10; //should be 0x00
    uint8_t sig11; //should be 0x55
    uint8_t sig12; //should be 0xAA   
}__attribute__((packed)) FAT32_FSInfo;

// should be 2 bytes total
typedef struct {
    uint8_t secs:5;
    uint8_t minutes:6;
    uint8_t hour:5;
}__attribute__((packed)) FAT32_FileTimeInfo;

// should be 3 bytes total
typedef struct {
    //file creation time information
    uint8_t tenths_sec:8; //valid: 0-199
    FAT32_FileTimeInfo time;
}__attribute__((packed)) FAT32_FileCreationTime;

// should be 2 bytes total
typedef struct {
    uint8_t day:5;
    uint8_t month:4;
    uint8_t year:7;
}__attribute__((packed)) FAT32_FileDateInfo;

typedef struct {
    //c string (first 8 chars are the file name and last 3 are extension)
    uint8_t file_name[11];
    uint8_t file_attributes;
    uint8_t reserved_windows_nt;

    // file creation information
    FAT32_FileCreationTime create_time;
    FAT32_FileDateInfo create_date;

    // last access info
    FAT32_FileDateInfo last_access_date;

    uint16_t cluster_num_high16;

    FAT32_FileTimeInfo last_modify_time;
    FAT32_FileDateInfo last_modify_date;

    uint16_t cluster_num_low16;
    uint32_t file_size_in_bytes;

}__attribute__((packed)) FAT32_Entry;

#define ENTRY_ATTR_READ_ONLY 0x01
#define ENTRY_ATTR_HIDDEN    0x02
#define ENTRY_ATTR_SYSTEM    0x04
#define ENTRY_ATTR_VOL_LABEL 0x08
#define ENTRY_ATTR_SUBDIR    0x10
#define ENTRY_ATTR_ARCHIVE   0x20
#define ENTRY_ATTR_DEVICE    0x40
#define ENTRY_ATTR_RESRVED   0x80

typedef struct {
    uint8_t entry_order;
    char entry_chars1[10];
    uint8_t attribute; //always 0x0F
    uint8_t long_entry_type; //0 for name entries
    uint8_t checksum;
    char entry_chars2[12];
    uint16_t zero; //always zero
    char entry_chars3[4];
}__attribute__((packed)) FAT32_LongEntry;

#define ENTRY_ORDER_NUMBER_MASK 0b011111
#define ENTRY_ORDER_LAST_LONG_ENTRY 0x40


typedef struct FS_Entry {
    char name[512];
    struct FS_Entry *children;
    struct FS_Entry *next;
} FS_Entry;

// helper functions
void readBPB(BPB *bpb);
unsigned int get_root_directory_sector(ExtendedFatBootRecord *efbr);

// functions for general kernel use

//todo replace this with fat32_probe
void initFAT32(void *params);

/* static  */SuperBlock *fat32_probe(BlockDev *dev);


void read_directory_entry(uint8_t *dir);

FS_Entry *read_directory_entries(BlockDev *dev, FAT32_SuperBlock *f32_sb, uint8_t *directory_block_num, int num_preceding_dirs, FS_Entry **incomplete_prev, FS_Entry *prev_entries);
FS_Entry *read_directory(BlockDev *dev, FAT32_SuperBlock *f32_sb, uint64_t directory_block_num, int num_preceding_dirs, FS_Entry **incomplete_prev, FS_Entry *prev_entries);

ino_t get_next_cluster(BlockDev *dev, FAT32_SuperBlock *f32_sb, FAT32_Inode *cur);
#endif
