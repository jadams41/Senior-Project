#ifndef PCI
#define PCI

/* universal pci configuration header type register offsets (in bytes) */
#define PCI_CONFIG_VENDOR_ID 0 //2 bytes
#define PCI_CONFIG_DEVICE_ID 2 //2 bytes
#define PCI_CONFIG_COMMAND 4 //2 bytes
#define PCI_CONFIG_STATUS 6 //2 bytes
#define PCI_CONFIG_REVIS_ID 8
#define PCI_CONFIG_PROG_IF 9
#define PCI_CONFIG_SUBCLASS 10
#define PCI_CONFIG_CLASS_CODE 11
#define PCI_CONFIG_CACHE_LINE_SIZE 12
#define PCI_CONFIG_LATENCY_TIMER 13
#define PCI_CONFIG_HEADER_TYPE 14

/* (type=00h) pci configuration header type register offsets (in bytes) */
#define PCI_CONFIG_BIST                    15
#define PCI_CONFIG_BAR0                    16 //4 bytes
#define PCI_CONFIG_BAR1                    20 //4 bytes
#define PCI_CONFIG_BAR2                    24 //4 bytes
#define PCI_CONFIG_BAR3                    28 //4 bytes
#define PCI_CONFIG_BAR4                    32 //4 bytes
#define PCI_CONFIG_BAR5                    36 //4 bytes
#define PCI_CONFIG_CARDBUS_CIS_POINTER     40 //4 bytes
#define PCI_CONFIG_SUBSYSTEM_VENDOR_ID     44 //2 bytes
#define PCI_CONFIG_SUBSYSTEM_ID            46 //2 bytes
#define PCI_CONFIG_EXPANSION_ROM_BASE_ADDR 48 //4 bytes
#define PCI_CONFIG_CAPABILITIES_PTR        52 
#define PCI_CONFIG_INTERRUPT_LINE          60 
#define PCI_CONFIG_INTERRUPT_PIN           61 
#define PCI_CONFIG_MIN_GRANT               62 
#define PCI_CONFIG_MAX_LATENCY             63

typedef struct IOBaseAddr IOBaseAddr;
typedef struct MemoryBaseAddr MemoryBaseAddr;
typedef struct PCIDevice PCIDevice;

struct IOBaseAddr {
    uint32_t base_addr;
    IOBaseAddr *next_io_addr;
};

struct MemoryBaseAddr {
    uint32_t base_addr;
    uint8_t prefetchable;
    uint8_t type;
    MemoryBaseAddr *next_mem_addr;
};

struct PCIDevice {
    /* information for interacting with the pci device */
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    
    /* information about the pci device */
    uint32_t vendor_id;
    uint32_t device_id;
    uint8_t rev_id;
    uint8_t class_code;
    
    IOBaseAddr *io_addrs;
    MemoryBaseAddr *mem_addrs;

    uint8_t interrupt_pin;
    uint8_t interrupt_line;
    
    /* linked list to next device */
    PCIDevice *next_device;
    
};

int pci_probe(PCIDevice **device_list);
uint32_t pci_config_read_field(uint8_t bus, uint8_t slot, uint8_t func, uint8_t field_offset, uint8_t field_size);
uint32_t pci_config_write_field(uint8_t bus, uint8_t slot, uint8_t func, uint8_t field_offset, uint32_t value, uint8_t value_len);
#endif
