#ifndef FAT_32
#define FAT_32

#include <stdint-gcc.h>

typedef struct {
    uint8_t inf_loop[3];
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t num_dir_entries;
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
    uint16_t fsinfo_sector_num;
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


void readBPB(BPB *bpb);
unsigned int get_root_directory_sector(ExtendedFatBootRecord *efbr);
void read_directory(uint8_t *dir);
#endif
