#ifndef _PIC_H
#define _PIC_H

/* PIC ports */
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

/* PIC commands */
#define PIC_EOI   0x20

#define PIC1_REMAPPED_START_VECTOR 0x20
#define PIC2_REMAPPED_START_VECTOR 0x28

/* standard IRQ lines */
//MASTER PIC IRQ LINES
#define SYSTEM_TIMER_IRQ          0
#define KEYBOARD_IRQ              1
#define SLAVE_PIC_IRQ             2
#define SERIAL_COM2_IRQ           3
#define SERIAL_COM4_IRQ           3 //shared with SERIAL_COM2
#define SERIAL_COM1_IRQ           4 
#define SERIAL_COM3_IRQ           4 //shared with SERIAL_COM1
#define PARALLEL_PORT2_IRQ        5
#define FLOPPY_DISK_IRQ           6
#define PARALLEL_PORT1_IRQ        7

//SLAVE PIC IRQ LINES
#define CMOS_RTC_IRQ              8
#define FREE_OR_NIC1_IRQ          9
#define FREE_OR_NIC2_IRQ          10
#define FREE_OR_NIC3_IRQ          11
#define MOUSE_IRQ                 12
#define FPU_IRQ                   13
#define PRIMARY_ATA_CHANNEL_IRQ   14
#define SECONDARY_ATA_CHANNEL_IRQ 15

int PIC_irq_from_pic1(int irq);
int PIC_irq_from_pic2(int irq);

void PIC_init();
void PIC_remap(uint8_t pic1_start, uint8_t pic2_start);

void PIC_mask_all_interrupts();
void PIC_disable_irq(int irq_line_num);
void PIC_enable_irq(int irq_line_num);
void PIC_end_of_interrupt(int irq);

void PIC_print_all_irq_status();
#endif
