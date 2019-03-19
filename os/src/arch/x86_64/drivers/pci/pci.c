#include <stdint-gcc.h>
#include "drivers/pci/pci.h"
#include "utils/printk.h"
#include "utils/utils.h"
#include "drivers/memory/memoryManager.h"

uint32_t pci_config_read_field(uint8_t bus, uint8_t slot, uint8_t func, uint8_t field_offset, uint8_t field_size){
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    //uint32_t full_register = 0;

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
	outb((uint8_t)value, 0xCFC + (field_offset & 3));
    }
    else if(value_len == 2){
	outw((uint16_t)value, 0xCFC + (field_offset & 2));
    }
    else if(value_len == 4){
	outl((uint32_t)value, 0xCFC);
    }
    
    return 0;
}

void add_bar_to_device(uint32_t bar, PCIDevice *dev){
    if(bar & 0b1){
	uint32_t base_addr = (bar & 0xFFFFFFFC);
	IOBaseAddr *new_addr = (IOBaseAddr*)kmalloc(sizeof(IOBaseAddr));

	new_addr->base_addr = base_addr;
	new_addr->next_io_addr = NULL;

	if(dev->io_addrs == NULL){
	    dev->io_addrs = new_addr;
	    return;
	}
	
	IOBaseAddr *cur_io_addr = dev->io_addrs;
	while(cur_io_addr->next_io_addr){
	    cur_io_addr = cur_io_addr->next_io_addr;
	}
	cur_io_addr->next_io_addr = new_addr;
	printk("I/O memory address: 0x%x\n", base_addr);
    }
    else {
	uint32_t base_addr = (bar & 0xFFFFFFF0) >> 4;
	//uint8_t prefetchable = (bar & 0b1000) >> 3;
	uint8_t type = (bar & 0b110) >> 1;
	printk("type 0x%x memory address: 0x%x\n", type, base_addr);
    }
}

PCIDevice *init_device(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendor_id){
    PCIDevice *new_dev = (PCIDevice*)kmalloc(sizeof(PCIDevice));

    uint16_t device_id = pci_config_read_field(bus, device, function, PCI_CONFIG_DEVICE_ID, 2);
    uint8_t rev_id = pci_config_read_field(bus, device, function, PCI_CONFIG_REVIS_ID, 1);
    uint8_t class_code = pci_config_read_field(bus, device, function, PCI_CONFIG_CLASS_CODE, 1);
    uint8_t header_type = pci_config_read_field(bus, device, function, PCI_CONFIG_HEADER_TYPE, 1);
    printk("-----------------------------------------------------------------\n");
    printk("vendor_id = 0x%x\n", vendor_id);
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
	
	add_bar_to_device(bar0, new_dev);
	add_bar_to_device(bar1, new_dev);
	add_bar_to_device(bar2, new_dev);
	add_bar_to_device(bar3, new_dev);
	add_bar_to_device(bar4, new_dev);
	add_bar_to_device(bar5, new_dev);

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

    return found_devices;
}
