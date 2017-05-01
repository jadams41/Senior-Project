#ifndef IDT
#define IDT

#define KEYBOARD_IRQ 1
#define SERIAL_COM2_IRQ 3
#define SERIAL_COM4_IRQ 3
#define SERIAL_COM1_IRQ 4
#define SERIAL_COM3_IRQ 4


void idt_init(void);
void IRQ_set_handler(int irq, void *handler);
void IRQ_set_mask(int IRQLine);
void IRQ_clear_mask(int IRQLine);
#endif
