#include <stdint-gcc.h>
#include "vfs.h"
#include "fat32.h"
#include "drivers/block/blockDeviceDriver.h"
#include "utils/printk.h"
#include "types/string.h"
#include "types/process.h"
#include "drivers/memory/memoryManager.h"

/* struct FSImpl {
   FS_detect_cb probe;
   struct FSImpl *next;
   };
   struct SuperBlock *FS_probe(struct BlockDev *dev);
*/

extern BlockDev *ata;

void readBPB(BPB *bpb){
    int i;

    printk("oemid='%s'\n", (char*)(bpb->oem_id));
    printk("bytes per sector=%d\n", bpb->bytes_per_sector);
    printk("sectors_per_cluster=%d\n", bpb->sectors_per_cluster);
    printk("num_reserved_sectors=%u\n", bpb->num_reserved_sectors);
    printk("num_fats=%u\n", bpb->num_fats);
    printk("num_directory_entires=%u\n", bpb->num_dir_entries);

    ExtendedFatBootRecord *efbr = (ExtendedFatBootRecord*)bpb;
    printk("sectors per fat=%u\n", efbr->sectors_per_fat); //ignored on fat32
    printk("fat version major=%d\n", efbr->fat_version_major);
    printk("fat version minor=%d\n", efbr->fat_version_minor);
    printk("root_dir_cluster_num=%u\n", efbr->root_dir_cluster_num);
    printk("fsinfo_sector_num=%hx\n", efbr->fsinfo_sector_num);
    printk("drive_num=%hx\n", efbr->drive_num);
    printk("signature (must be 0x28 or 0x29) = %hx\n", efbr->signature);

    printk("sys_id_str='");
    for(i = 0; i < 8; i++){
	printk("%c", ((char*)&(efbr->sys_id_str))[i]);
    }
    printk("'\n");

    printk("vol_label_str='");
    for(i = 0; i < 11; i++){
	printk("%c", ((char*)&(efbr->vol_label_str))[i]);
    }
    printk("'\n");

}

unsigned int get_root_directory_sector(ExtendedFatBootRecord *efbr){
    uint32_t sectors_per_cluster = efbr->bpb.sectors_per_cluster;
    uint32_t root_dir_cluster_num = efbr->root_dir_cluster_num;

    uint32_t root_dir_sector = sectors_per_cluster * (root_dir_cluster_num);

    return root_dir_sector;
}

void print_fat_date(FAT32_FileDateInfo *d){
    uint8_t month = d->month;
    uint8_t day = d->day;
    uint16_t year = d->year + 1980;
    
    printk("%d/%d/%d", month, day, year);
}

void print_fat_time(FAT32_FileTimeInfo *t){
    uint8_t hour = t->hour;
    uint8_t minutes = t->minutes;
    uint8_t secs = t->secs;
    printk("%d:%d:%d", hour, minutes, secs);
}

uint8_t *read_entry_name(FAT32_Entry *e, int preceding_le){
    printk("  - ");

    if(!preceding_le){
	int i;
	//print name
	for(i = 0; i < 8; i++){
	    char c = e->file_name[i];
	    if(c == 0){
		break;
	    }
	    if(e->file_attributes & FAT_ATTR_DIRECTORY){
		printk_info("%c", c);
	    }
	    else {
		printk_info("%c", c);
	    }
	}
	//print extension
	for(i = 8; i < 11; i++){
	    char c = e->file_name[i];
	    if(c == 0){
		break;
	    }
	    if(e->file_attributes & FAT_ATTR_DIRECTORY){
		printk_info("%c", c);
	    }
	    else {
		printk("%c", c);
	    }
	}
    }

    return (uint8_t*)(e + 1);
}



uint8_t *read_entry(FAT32_Entry *e, int preceding_le){
    if(!preceding_le){
	read_entry_name(e, 0);
    }

    //print creation datetime
    /* printk("    Created: "); */
    /* print_fat_date(&(e->create_date)); */
    /* printk(" "); */
    /* print_fat_time(&(e->create_time.time)); */
    /* printk(" (UTC)\n"); */

    /* //print modification datetime */
    /* printk("    Modified: "); */
    /* print_fat_date(&(e->last_modify_date)); */
    /* printk(" "); */
    /* print_fat_time(&(e->last_modify_time)); */
    /* printk(" (UTC)\n"); */
    
    /* printk("    File's first cluster: %u\n", (e->cluster_num_high16 << 16) + e->cluster_num_low16); */
    /* printk("    File size (bytes): %u\n", e->file_size_in_bytes); */

    return (uint8_t*)(e + 1);
}

uint8_t *read_long_entry(FAT32_LongEntry *le, int preceding_le){
    uint8_t *next_entry = ((uint8_t*)le) + 32, *last_entry = 0;

    if(next_entry[11] == 0x0F){
	last_entry = read_long_entry((FAT32_LongEntry *)next_entry, 1);
    }
    else {
	printk("  - ");
	last_entry = next_entry;
    }

    if(le->long_entry_type == 0){
	int done_reading = 0; //will set to 1 when null terminator is reached
	int i;

	// read first 5 characters in entry
	for(i = 0; i < 10; i += 2){
	    char c = le->entry_chars1[i];
	    if(c == 0){
		done_reading = 1;
		break;
	    }
	    printk("%c", c);
	}

	if(!done_reading){
	    //read next 6 characters in entry
	    for(i = 0; i < 12; i += 2){
		char c = le->entry_chars2[i];
		if(c == 0){
		    done_reading = 1;
		    break;
		}
		printk("%c", c);
	    }
	}

	if(!done_reading){
	    //read last 2 characters in entry
	    for(i = 0; i < 4; i += 2){
		char c = le->entry_chars3[i];
		if(c == 0){
		    done_reading = 1;
		    break;
		}
		printk("%c", c);
	    }
	}
	if(!preceding_le){
	    printk("\n");
	}
    }
    if(!preceding_le)
        return read_entry((FAT32_Entry*)last_entry, 1);
    return next_entry;
}

/*
 * loops through entries in the directory block
 */
void read_directory_entry(uint8_t *dir){
    uint8_t *entry = dir;
    while((entry - dir) < 512) {
	if(*entry == 0){
	    //printk("hit last entry\n");
	    break;
	}

	//not unused entry
	if(*entry != 0xE5){
	    //check long entry
	    if(entry[11] == 0x0F){
		entry = read_long_entry((FAT32_LongEntry*)entry, 0);
	    }
	    else {
		entry = read_entry((FAT32_Entry*)entry, 0);
	    }
	}
    }
}

/*
 * Function to read the BPB from disk and use that information to initialize
 * the FAT32 data structutes
 
 * NOTE: meant to be called inside a kernel process
*/
void initFAT32(void *params){
    if(params == 0){
	printk_err("did not pass parameters to initFAT32, needs reference to the ata device strucutre\n");
	kexit();
    }

    uint64_t *paramArr = params;
    int arrLen = *paramArr;
    if(arrLen - 1 != 1){
	printk_err("[initFAT32]: passed in the wrong number of parameters, expected 1 and received %d\n", arrLen - 1);
	kexit();
    }

    fat32_probe(ata);

    kexit();
}

void read_directory(BlockDev *dev, FAT32_SuperBlock *f32_sb, uint64_t directory_block_num, int first){
    char root_dir_block[512];
    
    // read in the root dir block
    ata->read_block(ata,  directory_block_num, root_dir_block);

    // parse entries in the root dir
    if(first){
	printk("--------- ROOT DIRECTORY (blocknum=%u) ---------\n", directory_block_num);
    }
    uint8_t *entry = (uint8_t*)root_dir_block;
    read_directory_entry(entry);

    FAT32_Inode cur;
    cur.inode.st_ino = directory_block_num - 2048 - f32_sb->first_data_sector + 2;
    ino_t next_cluster = get_next_cluster(dev, f32_sb, &cur); //14464;//= get_next_cluster(dev, f32_sb, &cur);
    
    if(next_cluster < 0x0FFFFFF8){
	uint64_t next_sector = (next_cluster * f32_sb->sectors_per_cluster) + f32_sb->first_data_sector - 2 + 2048;
	read_directory(dev, f32_sb, next_sector, 0);
    }
}

/* void create_dirent_inodes(BlockDev *dev, uint64_t dir_block_num, SuperBlock *parent_sb, Inode *parent){ */
/*     char dir_block[512]; */
/*     int i; */
/*     uint8_t *entry; */

/*     ata->read_block(ata, dir_block_num, dir_block); */
/*     entry = (uint8_t*)dir_block; */

/*     while(1) { */
/* 	//if reached end of block, but still more entries, read next dir_block */
/* 	if((entry - dir_block) >= 512){ */
/* 	    //todo assuming directory blocks are sequentially placed on disk, modify if this is incorrect */
/* 	    ata->read_block(ata, ++dir_block_num, dir_block); */
/* 	    entry = (uint8_t*)dir_block; */
/* 	} */

/* 	//reached end of directory entries, done */
/* 	if(*entry == 0){ */
/* 	    break; */
/* 	} */

/* 	//not unused entry */
/* 	if(*entry != 0xE5){ */
/* 	    char *filename[256]; //256 seems to be the maximum file length on FAT32 */
/* 	    int filename_len = 0, entry_filename_idx, done_reading_filename = 0; */

/* 	    while(entry[11] == 0x0F){ */
/* 		FAT32_LongEntry *le = (FAT32_LongEntry*)entry; */
/* 		char c; */
		
/* 		//read first 5 characters in LE */
/* 		for(entry_filename_idx = 0; entry_filename_idx < 10; entry_filename_idx += 2){ */
/* 		    c = le->entry_chars1[entry_filename_idx]; */
/* 		    filename[filename_len++] = c; */
/* 		    if(c == 0){ */
/* 			done_reading_filename = 1; */
/* 		    } */
/* 		} */

/* 		//read next 6 characters in LE */
/* 		for(entry_filename_idx = 0; !done_reading_filename && entry_filename_idx < 12; entry_filename_idx += 2){ */
/* 		    c = le->entry_chars2[entry_filename_idx]; */
/* 		    filename[filename_len++] = c; */
/* 		    if(c == 0){ */
/* 			done_reading_filename = 1; */
/* 		    } */
/* 		} */

/* 		//read last 2 characters in LE */
/* 		for(entry_filename_idx = 0; !done_reading_filename && entry_filename_idx < 4; entry_filename_idx += 2){ */
/* 		    c = le->entry_chars2[entry_filename_idx]; */
/* 		    filename[filename_len++] = c; */
/* 		    if(c == 0){ */
/* 			done_reading_filename = 1; */
/* 		    } */
/* 		} */

/* 		//go to next entry */
/* 		entry += sizeof(FAT32_LongEntry); */
/* 	    } */

/* 	    FAT32_Entry *e = (FAT32_Entry*)entry; */
	    
/* 	} */
/*     } */
/* } */

/* Inode *recursively_read_fs(BlockDev *dev, uint64_t file_block_num, int file_attributes, int file_size, SuperBlock *parent_sb, Inode *parent){ */
/*     Inode *inode = kmalloc(sizeof(Inode)); */
/*     inode->parent_super = (SuperBlock*)sb; */

/*     //if root_directory */
/*     if(parent == NULL){ */
/* 	inode->parent_inode = inode; */
/*     } */
    
/*     inode->st_ino = file_block_num; */
/*     inode->st_uid = 0; //todo figure out what to set this to */
/*     inode->st_gid = 0; //todo figure out what to set this to */
/*     inode->st_size = file_size; */
    
/*     if(block_is_dir){ */
/* 	inode->st_mode = FILE_DIR; */
/*     } */
/*     else { */
/* 	inode->st_mode = FILE_REGULAR */
/*     } */
/* } */

uint32_t get_next_cluster_num(FAT32_SuperBlock *sb, uint32_t cur_cluster_num){
    return 0;
}

void copy_file_allocation_table_into_sb(FAT32_SuperBlock *sb){
    
}

int read_fs_info_sector(BlockDev *dev, FAT32_SuperBlock *sb){
    char fsinfo_block[512];
    FAT32_FSInfo *fsinfo;
    uint32_t fsinfo_sector_num, total_volume_clusters;
    uint32_t last_known_free_clusters, last_allocated_data_cluster;
    int return_status = 0;

    total_volume_clusters = sb->total_sectors / sb->sectors_per_cluster;
    fsinfo_sector_num = sb->fsinfo_sector_num + 2048;

    //printk_info("Attempting to read FSInfo structure from sector %u\n", fsinfo_sector_num);
    
    if(fsinfo_sector_num > sb->total_sectors){
	printk_err("Invalid fsinfo sector number [%u] (volume only contains %u sectors)\n", fsinfo_sector_num, sb->total_sectors);

	sb->fsinfo_valid = 0;
	sb->last_known_free_clusters = 0;
	sb->last_allocated_data_cluster = 0;

	return 1;
    }
    
    memset(fsinfo_block, 0, 512);
    ata->read_block(ata, fsinfo_sector_num, fsinfo_block);

    fsinfo = (FAT32_FSInfo*)fsinfo_block;
    
    /* verify fsinfo is valid */
    if(fsinfo->sig1  != 0x52 ||
       fsinfo->sig2  != 0x52 ||
       fsinfo->sig3  != 0x61 ||
       fsinfo->sig4  != 0x41 ||
       fsinfo->sig5  != 0x72 ||
       fsinfo->sig6  != 0x72 ||
       fsinfo->sig7  != 0x41 ||
       fsinfo->sig8  != 0x61 ||
       fsinfo->sig9  != 0x00 ||
       fsinfo->sig10 != 0x00 ||
       fsinfo->sig11 != 0x55 ||
       fsinfo->sig12 != 0xAA) {
	printk_warn("Invalid FSInfo Block (bad signatures)\n");

	sb->fsinfo_valid = 0;
	sb->last_known_free_clusters = 0;
	sb->last_allocated_data_cluster = 0;

	return 1;
    }

    /* seemingly valid FSInfo Sector */
    sb->fsinfo_valid = 1;

    /* get last known number of free data clusters on the volume */
    last_known_free_clusters = fsinfo->last_known_free_clusters;
    
    if(last_known_free_clusters == 0xFFFFFFFF){
	printk_info("FSInfo unaware of the remaining free clusters on the volume\n");

	//todo see if this should be discovered and set (by looping over the entire FAT perhaps?)

	sb->last_known_free_clusters = 0;
    }

    //make sure last_known_free_clusters is > 0 and < number of sectors on the voluem
    else if(last_known_free_clusters > total_volume_clusters){
	printk_warn("FSInfo contains suspicious value of remaining free clusters=%u (total number of clusters=%u). Ignoring.\n",
		    last_known_free_clusters, total_volume_clusters);
	
	//todo see if this value should be discovered and set
	sb->last_known_free_clusters = 0;
	return_status = 2;
    }
    else {
	sb->last_known_free_clusters = last_known_free_clusters;
	printk_info("Last known number of free clusters in the volume=%u\n", last_known_free_clusters);
    }

    /* get last allocated data cluster on the volume */
    last_allocated_data_cluster = fsinfo->last_allocated_data_cluster;

    //0xFFFFFFFF indicates that there hasn't been a data cluster allocated since formatting
    if(last_allocated_data_cluster == 0xFFFFFFFF){
	printk_info("There have been no data clusters allocated since volume was formatted\n");
    }
    else if(last_allocated_data_cluster > total_volume_clusters){
	printk_warn("FSInfo contains suspicious value of last allocated data cluster [%u] (total number of clusters is %u)\n",
		    last_allocated_data_cluster, total_volume_clusters);

	//todo figure out if this value should/can be repaired and set
	sb->last_allocated_data_cluster = 0;
	
	return_status = 2;
    }
    else {
	sb->last_allocated_data_cluster = last_allocated_data_cluster;
	printk_info("Last data cluster allocated in the volume is %u\n", last_allocated_data_cluster);
    }


    return return_status;
}

FAT32_Inode *create_inode(ino_t st_ino, mode_t st_mode, uid_t st_uid, gid_t st_gid, off_t st_size, uint8_t attrs){
    FAT32_Inode *new_f32_inode = (FAT32_Inode*)kmalloc(sizeof(FAT32_Inode));
    Inode *new_inode = &(new_f32_inode->inode);
    
    // set Inode attributes
    new_inode->st_ino = st_ino;
    new_inode->st_mode = st_mode;
    new_inode->st_uid = st_uid;
    new_inode->st_gid = st_gid;
    new_inode->st_size = st_size;

    new_f32_inode->attributes = attrs;

    return new_f32_inode;
}

/* Verifies the information contained in the supplied FAT32 BPB
 * Note: Information about legal values taken from: https://infogalactic.com/info/Design_of_the_FAT_file_system#BPB20 */
int verify_fat32_bpb(BPB *bpb){
    int return_status = 0;
    
    /* INF Loop Instructions
       first three bytes should be 0xEB 0x3C? 0x90 (JMP SHORT, 3C, NOP) */
    if(bpb->inf_loop[0] != 0xEB || bpb->inf_loop[2] != 0x90){
	printk_err("Invalid bpb inf loop instructions %x %x %x (should be 0xEB 0x3C? 0x90)\n", bpb->inf_loop[0], bpb->inf_loop[1], bpb->inf_loop[2]);
	return_status = 1;
    }

    /* Bytes Per Sector 
       Most common value is 512. 
       Minimum logical sector size for standard FAT32 volumes is 512 bytes, which can be reduced down to 128 bytes without support for the FS Information Sector. */
    uint16_t bytes_per_sector = bpb->bytes_per_sector;
    if(bytes_per_sector < 128){
	printk_err("Invalid bpb bytes_per_sector value %u\n", bytes_per_sector);
	return_status = 1;
    }

    /* Logical Sectors per Cluster
       Allowed values are 1, 2, 4, 8, 16, 32, 64, and 128. */
    uint8_t sectors_per_cluster = bpb->sectors_per_cluster;
    if(sectors_per_cluster != 1  &&
       sectors_per_cluster != 2  &&
       sectors_per_cluster != 4  &&
       sectors_per_cluster != 8  &&
       sectors_per_cluster != 16 &&
       sectors_per_cluster != 32 &&
       sectors_per_cluster != 64 &&
       sectors_per_cluster != 128){
	printk_err("Invalid bpb sectors_per_cluster value %u\n", sectors_per_cluster);
	return_status = 1;
    }

    /* Count of Reserved Logical sectors
       At least 1 for this sector. 
       Usually 32 for FAT32 (to hold the extended boot sector, FS info sector and backup boot sectors) */
    uint16_t num_reserved_sectors = bpb->num_reserved_sectors;
    if(num_reserved_sectors < 1){
	printk_err("Invalid bpb num_reserved_sectors value %u\n", num_reserved_sectors);
	return_status = 1;
    }

    /* Number of File Allocation Tables (FATs)
       Almost always 2 */
    uint8_t num_fats = bpb->num_fats;
    if(num_fats < 1){
	printk_err("Invalid bpb number_file_allocation_tables %u\n", num_fats);
	return_status = 1;
    }

    /* Maximum number of FAT12 or FAT16 root entries
       0 for FAT32, where the root directory is stored in ordinary data clusters */
    uint16_t num_dir_entries = bpb->num_dir_entries;
    if(num_dir_entries != 0){
	printk_err("Invalid bpb num_dir_entries value %u\n", num_dir_entries);
	return_status = 1;
    }

    /* Total logical sectors
       if zero, use 4 byte value at offset 0x020 */
    uint16_t total_sectors = *((uint16_t*)bpb->total_sectors);
    uint32_t large_sector_count = *((uint32_t*)bpb->large_sector_count);
    if(total_sectors == 0 && large_sector_count == 0){
	printk_err("Invalid bpb total logical sectors (both total_sectors and large_sector_count are zero)\n");
	return_status = 1;
    }

    /* Logical sectors per File Allocation Table
       0 for FAT32 */
    uint16_t sectors_per_fat = bpb->sectors_per_fat;
    if(sectors_per_fat != 0){
	printk_err("Invalid bpb sectors_per_fat value %u (should be 0 for FAT32)\n", sectors_per_fat);
	return_status = 1;
    }

    return return_status;
}

// returns 0 if valid
int verify_fat32_efbr(ExtendedFatBootRecord *efbr){
    int return_status = 0;
    uint32_t total_sectors = 0;
    uint16_t total_sectors_short = *((uint16_t*)efbr->bpb.total_sectors);
    if(total_sectors_short == 0){
	total_sectors = *((uint32_t*)efbr->bpb.large_sector_count);
    }
    else {
	total_sectors = total_sectors_short;
    }
    uint64_t total_clusters = total_sectors / efbr->bpb.sectors_per_cluster;

    printk("total_sectors = %u, total_clusters = %u\n", total_sectors, total_clusters);
    
    /* Logical Sectors per File allocation table */

    /* Drive description / mirroring flags */

    /* Version
       FAT32 implementations should refuse to mount volumes with version numbers unknown by them (todo)*/

    /* Cluster number of the root directory
       A cluster value of 0 is not officially allowed and can never indicate a valid root directory start cluster */
    uint32_t root_dir_cluster_num = efbr->root_dir_cluster_num;
    if(root_dir_cluster_num == 0){
	printk_err("Invalid efbr root directory cluster number %u (can't be 0)\n");
	return_status = 1;
    }
    else if(root_dir_cluster_num >= total_clusters){
	printk_err("Invalid efbr root directory cluster number %u (greater than total clusters %u)\n", root_dir_cluster_num, total_clusters);
	return_status = 1;
    }

    /* Logical sector number of FS Information Sector
       typically 1 (the second of the three FAT32 boot sectors */
    uint16_t fsinfo_sector_num = efbr->fsinfo_sector_num;
    if(fsinfo_sector_num == 0){
	printk_err("Invalid efbr FSInfo Sector value %u\n", fsinfo_sector_num);
	return_status = 1;
    }
    else if(fsinfo_sector_num >= total_sectors){
	printk_err("Invalid efbr FSInfo Sector value %u (greater than total sectors &u)\n", fsinfo_sector_num, total_sectors);
	return_status = 1;
    }

    /* First Logical sector number of a copy of the three FAT32 boot sectors */

    /* Reserved */

    /* Physical drive number */

    /* Windows NT Flags */

    /* Extended Boot Signature
       Should be 0x28 or 0x29 */
    uint8_t signature = efbr->signature;
    if(signature != 0x28 && signature != 0x29){
	printk_err("Invalid efbr Signature %u (should be either 0x28 or 0x29)\n", signature);
	return_status = 1;
    }
    
    /* Volume ID */

    /* Volume Label
       Not avaiable if signature is 0x28 */

    /* SYS ID STR */

    /* Bootable Partition Sign */

    return return_status;
}

ino_t get_next_cluster(BlockDev *dev, FAT32_SuperBlock *f32_sb, FAT32_Inode *cur){
    uint64_t num_entries_per_fat_sector, last_fat_sector;
    uint32_t *fat_sector, cur_cluster_num, relevant_fat_sector_num;

    ino_t next_ino;

    //the cluster we are going to find the next cluster for
    cur_cluster_num = cur->inode.st_ino;
    
    //figure out how many entries are in each FAT sector
    num_entries_per_fat_sector = f32_sb->bytes_per_sector / sizeof(uint32_t);
    fat_sector = kmalloc(sizeof(uint32_t) * num_entries_per_fat_sector);

    //printk_info("num_entries_per_fat_sector = %u\n", num_entries_per_fat_sector);

    //get the end of the last File Allocation Table (to make sure that we don't read past the end)
    last_fat_sector = f32_sb->first_fat_sector + (f32_sb->num_fats * f32_sb->sectors_per_fat);
    
    //find the sector number of the File Allocation Table portion which contains the entry for the current cluster
    relevant_fat_sector_num = f32_sb->first_fat_sector + (cur_cluster_num / num_entries_per_fat_sector);

    //printk_info("inode %u has FAT entry in sector %u\n", cur_cluster_num, relevant_fat_sector_num);

    //make sure the relevant fat sector is valid
    if(relevant_fat_sector_num >= last_fat_sector){
	printk_err("attempt to get next cluster for %u resulted in illegal File Allocation Table sector %u\n", cur_cluster_num, relevant_fat_sector_num);
    }


    ata->read_block(ata, relevant_fat_sector_num, fat_sector);

    next_ino = fat_sector[cur_cluster_num % (num_entries_per_fat_sector)];

    kfree(fat_sector);
			  
    return next_ino;
}

/* void read_directory(BlockDev *dev, FAT32_SuperBlock *f32_sb, FAT32_Inode *dir, int read_subdirectories){ */
/*     uint8_t dir_block[512]; */
/*     uint8_t *entry; */
    
/*     ata->read_block(ata, dir->st_ino, dir_block); */
/*     entry = (uint8_t*) */
	
/*     while(1){ */
/* 	//need to get the next cluster for this directory */
/* 	if(entry >= dir_block){ */
/* 	    //get next cluster from cluster chain for the directory */
/* 	    ino_t next_dir_cluster = get_next_cluster(dev, f32_sb, dir); */
/* 	    dir = create_inode(next_dir_cluster, 0, 0, 0, 0, 0); */
/* 	} */
/*     } */
/* } */

/* void traverse_directory_tree(BlockDev *dev, FAT32_SuperBlock *f32_sb){ */
/*     Fat32_Inode *root_inode = (FAT32_Inode*)f32_sb->super.root_inode; */
    
/*     read_directory(dev, root_inode, 1); */
/* } */


/* static */SuperBlock *fat32_probe(BlockDev *dev){
    char efbr_block[512];
    BPB *bpb;
    ExtendedFatBootRecord *efbr;
    FAT32_SuperBlock *f32_sb = (FAT32_SuperBlock*)kmalloc(sizeof(FAT32_SuperBlock));
    FAT32_Inode *root_inode;
    uint64_t root_dir_sector, root_dir_sectors, first_data_sector;
    uint32_t fat_size, large_sector_count;
    uint16_t total_sectors;
    
    // read in efbr_block
    memset(efbr_block, 0, 512);
    ata->read_block(ata, 2048, efbr_block); //todo replace 2048 with dynamic information from somewhere

    efbr = (ExtendedFatBootRecord*)efbr_block;
    bpb = &(efbr->bpb);
    if(verify_fat32_bpb(bpb) || verify_fat32_efbr(efbr)){
	printk_err("Invalid FAT32 efbr or bpb, not creating superblock\n");
	return NULL;
    }
    
    readBPB(&(efbr->bpb));

    root_dir_sector = get_root_directory_sector((ExtendedFatBootRecord*)efbr_block);

    fat_size = efbr->sectors_per_fat;
    root_dir_sectors = 0; //0 on FAT32
    first_data_sector = efbr->bpb.num_reserved_sectors + (efbr->bpb.num_fats * fat_size) + root_dir_sectors;

    // initialize fs superblock
    f32_sb->bytes_per_sector = efbr->bpb.bytes_per_sector;
    f32_sb->sectors_per_cluster = efbr->bpb.sectors_per_cluster;
    f32_sb->num_reserved_sectors = efbr->bpb.num_reserved_sectors;
    f32_sb->num_fats = efbr->bpb.num_fats;
    f32_sb->first_fat_sector = f32_sb->num_reserved_sectors + 2048; //wiki.osdev.org/FAT
    f32_sb->sectors_per_fat = efbr->sectors_per_fat;

    
    f32_sb->fsinfo_sector_num = efbr->fsinfo_sector_num;

    total_sectors = *((uint16_t*)efbr->bpb.total_sectors);
    large_sector_count = *((uint32_t*)efbr->bpb.large_sector_count);
    
    f32_sb->total_sectors = total_sectors != 0 ? total_sectors : large_sector_count;
    f32_sb->num_hidden_sectors = *((uint8_t*)efbr->bpb.num_hidden_sectors);

    f32_sb->root_dir_sector = root_dir_sector;
    f32_sb->first_data_sector = first_data_sector;

    read_fs_info_sector(dev, f32_sb);

    //create inode for the root directory
    root_inode = create_inode(root_dir_sector, 0, 0, 0, 0, 0);
    f32_sb->super.root_inode = (Inode*)root_inode;
    
    //set superblock functions
    //todo

    ino_t next_ino = get_next_cluster(dev, f32_sb, root_inode);
    printk("root directory sector = %u, next sector = %lu\n", f32_sb->root_dir_sector, next_ino);
    
    read_directory(dev, f32_sb, root_dir_sector - 2 + first_data_sector + 2048, 1);
    
    return (SuperBlock*)f32_sb;
}
