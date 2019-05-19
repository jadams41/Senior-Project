#include <stdint-gcc.h>
#include "drivers/pci/pci.h"
#include "utils/printk.h"
#include "utils/utils.h"
#include "drivers/memory/memoryManager.h"
#include "drivers/pci/vendors.h"

uint32_t pci_config_read_field(uint8_t bus, uint8_t slot, uint8_t func, uint8_t field_offset, uint8_t field_size){
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    if(((field_offset % 4) + field_size) > 4){
	return 0;
    }
    
    /* create configuration address */
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (field_offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(0xCF8, address);

    if(field_size == 1){
	return inb(0xCFC + (field_offset & 3));
    }
    else if(field_size == 2){
	return inw(0xCFC + (field_offset & 2));
    }
    else if(field_size == 4){
	return inl(0xCFC);
    }
    else {
	printk_err("bad usage, field size %d is not allowed\n", field_size);
	asm("hlt");
    }
    
    return 0;
}

uint32_t pci_config_write_field(uint8_t bus, uint8_t slot, uint8_t func, uint8_t field_offset, uint32_t value, uint8_t value_len){
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (field_offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(0xCF8, address);

    if(value_len == 1){
	outb(0xCFC + (field_offset & 3), (uint8_t)value);
    }
    else if(value_len == 2){
	outw(0xCFC + (field_offset & 2), (uint16_t)value);
    }
    else if(value_len == 4){
	outl(0xCFC, (uint32_t)value);
    }
    else {
	printk_err("bad usage, value_len %d is not allowed\n", value_len);
	asm("hlt");
    }
    
    return 0;
}

/**
 * Determine the amount of address space required for the bar
 * Follows PCI 2.2 Implementation Note: Sizing a 32 bit Base Address Register:
 *   1) Decode of a register is disabled via the command register before sizing
 *   2) Save the original value of BARX register
 *   3) Write all 1s to BARX register (0xFFFFFFFF)
 *   4) Read back BARX register
 *      a) clear encoding information bits (bit 0 for I/O, bits 0-3 for Mem)
 *      b) invert all bits
 *      c) increment by 1
 *   5) Write back original value 
 *   6) Reenable decode in command register */
uint32_t decode_bar(uint32_t bar_val, uint32_t bar_offset, PCIDevice *dev){
    uint32_t all_1s = 0xFFFFFFFF;
    uint16_t cmd = pci_config_read_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, 2);
    uint32_t mem_needed;

    /* I/O bar type */
    if(bar_val & 0b1){
	/* disable I/O register decode (cmd register bit 0) */
	cmd &= 0xFFFE;
	pci_config_write_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, cmd, 2);

	/* write all 1s to BARX register */
	pci_config_write_field(dev->bus, dev->slot, dev->func, bar_offset, all_1s, 4);

	/* read back BARX */
	mem_needed = pci_config_read_field(dev->bus, dev->slot, dev->func, bar_offset, 4);
	
	/* calculate mem_needed */
	// clear all encoding information bits (bit 0 for I/O)
	mem_needed &= 0xFFFFFFFE;
	// invert all bits
	mem_needed = ~mem_needed;
	//increment by 1
	mem_needed += 1;

	/* write back original value */
	pci_config_write_field(dev->bus, dev->slot, dev->func, bar_offset, bar_val, 4);

	/* reenable I/O register decode (cmd register bit 0) */
	cmd |= 0b1;
        pci_config_write_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, cmd, 2);
    }
    /* Mem bar type */
    else {
	/* disable Mem register decode (cmd register bit 1) */
	cmd &= 0xFFFE;
	pci_config_write_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, cmd, 2);

	/* write all 1s to BARX register */
	pci_config_write_field(dev->bus, dev->slot, dev->func, bar_offset, all_1s, 4);

	/* read back BARX */
	mem_needed = pci_config_read_field(dev->bus, dev->slot, dev->func, bar_offset, 4);

	/* calculate mem_needed */
	// clear all encoding information bits (bits 0-3 for I/O)
	mem_needed &= 0xFFFFFFF0;
	// invert all bits
	mem_needed = ~mem_needed;
	//increment by 1
	mem_needed += 1;

	/* write back original value */
	pci_config_write_field(dev->bus, dev->slot, dev->func, bar_offset, bar_val, 4);

	/* reenable I/O register decode (cmd register bit 1) */
	cmd |= 0b10;
	//cmd |= 0b1;
        pci_config_write_field(dev->bus, dev->slot, dev->func, PCI_CONFIG_COMMAND, cmd, 2);
    }

    return mem_needed;
}

void add_bar_to_device(uint32_t bar_val, uint32_t bar_offset, PCIDevice *dev){
    /* Unimplemented Base Address Registers are hardwired to zero */
    if(bar_val == 0){
	return;
    }
    if(bar_val & 0b1){
	uint32_t base_addr = (bar_val & 0xFFFFFFFC);
	IOBaseAddr *new_addr = (IOBaseAddr*)kmalloc(sizeof(IOBaseAddr));

	new_addr->base_addr = base_addr;
	new_addr->mem_needed = decode_bar(bar_val, bar_offset, dev);
	new_addr->next_io_addr = NULL;

	if(dev->io_addrs == NULL){
	    dev->io_addrs = new_addr;
	    printk("I/O memory address: 0x%x (size=0x%x)\n", base_addr, new_addr->mem_needed);
	    return;
	}
	
	IOBaseAddr *cur_io_addr = dev->io_addrs;
	while(cur_io_addr->next_io_addr){
	    cur_io_addr = cur_io_addr->next_io_addr;
	}
	cur_io_addr->next_io_addr = new_addr;
	printk("I/O memory address: 0x%x (size=0x%x)\n", base_addr, new_addr->mem_needed);
    }
    else {
	uint32_t base_addr = (bar_val & 0xFFFFFFF0) >> 4;
	//uint8_t prefetchable = (bar & 0b1000) >> 3;
	uint8_t type = (bar_val & 0b110) >> 1;
	uint32_t mem_needed = decode_bar(bar_val, bar_offset, dev);
	printk("type 0x%x memory address: 0x%x (size=0x%x)\n", type, base_addr, mem_needed);
    }
}

PCIDevice *init_device(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendor_id){
    PCIDevice *new_dev = (PCIDevice*)kmalloc(sizeof(PCIDevice));

    uint16_t device_id = pci_config_read_field(bus, device, function, PCI_CONFIG_DEVICE_ID, 2);
    uint8_t rev_id = pci_config_read_field(bus, device, function, PCI_CONFIG_REVIS_ID, 1);
    uint8_t class_code = pci_config_read_field(bus, device, function, PCI_CONFIG_CLASS_CODE, 1);
    uint8_t header_type = pci_config_read_field(bus, device, function, PCI_CONFIG_HEADER_TYPE, 1);
    printk("-----------------------------------------------------------------\n");
    printk("vendor_id = 0x%x [%s]\n", vendor_id, lookup_vendor_id(vendor_id));
    printk("device_id = 0x%x\n", device_id);
    new_dev->bus = bus;
    new_dev->slot = device;
    new_dev->func = function;

    new_dev->vendor_id = vendor_id;
    new_dev->device_id = device_id;
    new_dev->rev_id = rev_id;
    new_dev->class_code = class_code;
    
    new_dev->io_addrs = NULL;
    new_dev->mem_addrs = NULL;
    
    if(header_type == 0x00){	
	uint32_t bar0 = pci_config_read_field(bus, device, function, PCI_CONFIG_BAR0, 4);
	uint32_t bar1 = pci_config_read_field(bus, device, function, PCI_CONFIG_BAR1, 4);
	uint32_t bar2 = pci_config_read_field(bus, device, function, PCI_CONFIG_BAR2, 4);
	uint32_t bar3 = pci_config_read_field(bus, device, function, PCI_CONFIG_BAR3, 4);
	uint32_t bar4 = pci_config_read_field(bus, device, function, PCI_CONFIG_BAR4, 4);
	uint32_t bar5 = pci_config_read_field(bus, device, function, PCI_CONFIG_BAR5, 4);
	
	uint8_t interrupt_pin = pci_config_read_field(bus, device, function, PCI_CONFIG_INTERRUPT_PIN, 1);
	uint8_t interrupt_line = pci_config_read_field(bus, device, function, PCI_CONFIG_INTERRUPT_LINE, 1);
	
	add_bar_to_device(bar0, PCI_CONFIG_BAR0, new_dev);
	add_bar_to_device(bar1, PCI_CONFIG_BAR1, new_dev);
	add_bar_to_device(bar2, PCI_CONFIG_BAR2, new_dev);
	add_bar_to_device(bar3, PCI_CONFIG_BAR3, new_dev);
	add_bar_to_device(bar4, PCI_CONFIG_BAR4, new_dev);
	add_bar_to_device(bar5, PCI_CONFIG_BAR5, new_dev);

	new_dev->interrupt_pin = interrupt_pin;
	new_dev->interrupt_line = interrupt_line;
	printk("interrupt_pin: 0x%x\n", interrupt_pin);
	printk("interrupt_line: 0x%x\n", interrupt_line);
    }

    return new_dev;
}

PCIDevice *check_device_funcs(uint8_t bus, uint8_t device){
    PCIDevice *new_devices = NULL;
    PCIDevice *cur_device = NULL;
    uint8_t function = 0;
    uint8_t header_type;
    uint16_t vendor_id = pci_config_read_field(bus, device, function, PCI_CONFIG_VENDOR_ID, 2);

    //use vendor id to test (bus, device), if device not available, will return 0xFFFF for any offset
    if(vendor_id == 0xFFFF){
	return new_devices;
    }

    cur_device = new_devices = init_device(bus, device, function, vendor_id);
    
    header_type = pci_config_read_field(bus, device, function, PCI_CONFIG_HEADER_TYPE, 1);

    //check if it is a multi-function device
    if((header_type & 0x80) != 0){
	for(function = 1; function < 8; function++){
	    vendor_id = pci_config_read_field(bus, device, function, PCI_CONFIG_VENDOR_ID, 2);

	    if(vendor_id != 0xFFFF){
	        cur_device->next_device = init_device(bus, device, function, vendor_id);
		cur_device = cur_device->next_device;
	    }
	}
    }

    return new_devices;
}


int pci_probe(PCIDevice **device_list) {
    PCIDevice *cur_device = NULL;
    int found_devices = 0;
    
    uint16_t bus;
    uint8_t device;

    for(bus = 0; bus < 256; bus++){
	for(device = 0; device < 32; device++){
	    PCIDevice *new_devices = check_device_funcs(bus, device);
	    if(new_devices){
		if(!*device_list){
		    cur_device = *device_list = new_devices;
		    found_devices += 1;
		}
		else {
		    cur_device->next_device = new_devices;
		    found_devices += 1;
		}

		while(cur_device->next_device != NULL){
		    cur_device = cur_device->next_device;
		    found_devices += 1;
		}
	    }
	}
    }

    if(found_devices){
	printk("-----------------------------------------------------------------\n");
    }
    return found_devices;
}
