#include "vendors.h"
#include <stdint-gcc.h>

char *lookup_vendor_id(uint16_t vendor_id) {
    switch(vendor_id){
    case 0x1000:
	return "NCR";
    case 0x1002:
	return "ATI Technologies";
    case 0x100B:
	return "National Semiconductor Corporation";
    case 0x1013:
	return "Cirrus Logic";
    case 0x1022:
	return "Advanced Micro Devices";
    case 0x102B:
	return "Matrox Graphics, Inc.";
    case 0x1039:
	return "Silicon Integrated Systems (SiS)";
    case 0x104C:
	return "Texas Instruments";
    case 0x105A:
	return "Promise Technology";
    case 0x10B7:
	return "3Com Corporation";
    case 0x10B9:
	return "AcerLabs (ALI)";
    case 0x10DE:
	return "nVidia Corporation";
    case 0x10EC:
	return "Realtek";
    case 0x1106:
	return "VIA";
    case 0x110A:
	return "Siemens Nixdorf AG";
    case 0x1234:
	return "Qemu";
    case 0x125D:
	return "ESS Technology";
    case 0x1274:
	return "Ensoniq";
    case 0x5333:
	return "S3";
    case 0x8086:
	return "Intel";
    case 0x9004:
    case 0x9005:
	return "Adaptec";
    }
    
    return "Unknown Vendor";
}
