#ifndef VENDORS
#define VENDORS

#include <stdint-gcc.h>

//http://www.doc.ic.ac.uk/~svb/CS/Lab/Minix%203.1.0/drivers/libpci/pci_table.c

char *lookup_vendor_id(uint16_t vendor_id);
char *lookup_device_id(uint16_t vendor_id, uint16_t device_id);

#endif
