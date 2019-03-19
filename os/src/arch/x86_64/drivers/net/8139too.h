#ifndef _8139TOO
#define _8139TOO

#include "drivers/pci/pci.h"

typedef struct rt8139_private {
    PCIDevice *dev;

    uint64_t rx_buf_virt;
    uint32_t rx_buf_phys;
} rt8139_private;

int init_rt8139(PCIDevice *dev);
#endif
