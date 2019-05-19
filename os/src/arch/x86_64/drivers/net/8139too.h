#ifndef _8139TOO
#define _8139TOO

#include "drivers/pci/pci.h"

#define NUM_TX_DESC 4
#define MAX_ETH_FRAME_SIZE 1792

#define TX_BUF_SIZE MAX_ETH_FRAME_SIZE
#define TX_BUF_TOT_LEN (TX_BUF_SIZE * NUM_TX_DESC)

#define RX_FIFO_THRESH	7	/* Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST	7	/* Maximum PCI burst, '6' is 1024 */
#define TX_DMA_BURST	6	/* Maximum PCI burst, '6' is 1024 */
#define TX_RETRY	8	/* 0-15.  retries = 16 + (TX_RETRY * 16) */

typedef struct rt8139_private {
    PCIDevice *dev;

    uint64_t rx_buf_virt;
    uint32_t rx_buf_phys;
    uint64_t rx_buf_size;
    uint64_t cur_rx; //keeps track of current position inside rx ring buffer
    
    unsigned char *tx_buf[NUM_TX_DESC];
    unsigned char *tx_bufs;
    uint32_t tx_buf_phys;
} rt8139_private;

int init_rt8139(PCIDevice *dev);
#endif
