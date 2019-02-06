#include <stdint-gcc.h>
#include "fat32.h"
#include "utils/printk.h"

void readBPB(BPB *bpb){
    int i;

    printk("oemid='%s'\n", (char*)(bpb->oem_id));
    printk("bytes per sector=%d\n", bpb->bytes_per_sector);
    printk("sectors_per_cluster=%d\n", bpb->sectors_per_cluster);
    printk("num_reserved_sectors=%u\n", bpb->num_reserved_sectors);
    printk("num_fats=%u\n", bpb->num_fats);

    ExtendedFatBootRecord *efbr = (ExtendedFatBootRecord*)bpb;
    //printk("sectors per fat=%u\n", efbr->sectors_per_fat); //ignored on fat32
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
	    printk("%c", c);
	}
	//print extension
	for(i = 8; i < 11; i++){
	    char c = e->file_name[i];
	    if(c == 0){
		break;
	    }
	    printk("%c", c);
	}
    }

    return (uint8_t*)(e + 1);
}



uint8_t *read_entry(FAT32_Entry *e, int preceding_le){
    if(!preceding_le){
	read_entry_name(e, 0);
    }
    //print creation datetime
    printk("    Created: ");
    print_fat_date(&(e->create_date));
    printk(" ");
    print_fat_time(&(e->create_time.time));
    printk(" (UTC)\n");

    //print modification datetime
    printk("    Modified: ");
    print_fat_date(&(e->last_modify_date));
    printk(" ");
    print_fat_time(&(e->last_modify_time));
    printk(" (UTC)\n");
    
    printk("    File's first cluster: %u\n", (e->cluster_num_high16 << 16) + e->cluster_num_low16);
    printk("    File size (bytes): %u\n", e->file_size_in_bytes);

    
    
    
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

void read_directory(uint8_t *dir){
    printk("size entry=%d, size long_entry=%d\n", sizeof(FAT32_Entry), sizeof(FAT32_LongEntry));
    uint8_t *entry = dir;
    while((entry - dir) < 512) {
	if(*entry == 0){
	    printk("hit last entry\n");
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
