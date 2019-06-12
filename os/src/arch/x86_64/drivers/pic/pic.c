#include <stdint-gcc.h>
#include "drivers/pic/pic.h"
#include "utils/printk.h"
#include "utils/utils.h"

static uint8_t pics_initialized  = 0;
static uint8_t pic1_vector_start = 0;
static uint8_t pic2_vector_start = 0;
static uint8_t pic1_vector_end   = 0;
static uint8_t pic2_vector_end   = 0;

int PIC_irq_from_pic1(int irq){
	if(!pics_initialized){
		printk_err("PICs not initialized yet, can't check if interrupt came from pic1\n");
		return 0;
	}
	return irq >= pic1_vector_start && irq <= pic1_vector_end;
}

int PIC_irq_from_pic2(int irq){
	if(!pics_initialized){
		printk_err("PICs not initialized yet, can't check if interrupt came from pic2\n");
		return 0;
	}
	return irq >= pic2_vector_start && irq <= pic2_vector_end;
}

void PIC_init(){
	if(pics_initialized){
		printk_err("PICs already initialized, use PIC_remap instead\n");
		return ;
	}
	
	PIC_remap(PIC1_REMAPPED_START_VECTOR, PIC2_REMAPPED_START_VECTOR);

	pics_initialized = 1;
}

void PIC_remap(uint8_t pic1_start, uint8_t pic2_start){
	/* restart the PICs */
	outb(PIC1_CMD, 0x11);
	outb(PIC2_CMD, 0x11);
	
	/* remap the PICs */
	outb(PIC1_DATA, pic1_start);
	outb(PIC2_DATA, pic2_start);
	
	/* setup cascading */
	outb(PIC1_DATA, 0x04); //tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(PIC2_DATA, 0x02); //tell Slave PIC its cascade identity (0000 0010) 
	
	/* finish */
	outb(PIC1_DATA, 0x01);
	outb(PIC2_DATA, 0x01);

	/* update the PIC configuration variables */
	pic1_vector_start = pic1_start;
	pic1_vector_end   = pic1_start + 7;
	
	pic2_vector_start = pic2_start;
	pic2_vector_end   = pic2_start + 7;
}

void PIC_mask_all_interrupts(){
	if(!pics_initialized){
		printk_err("can't modify PICs without having initialized them first\n");
		return;
	}
	
	/* mask all interrupts */
	outb(PIC1_DATA, 0xff);
	outb(PIC2_DATA, 0xff);

	printk_info("All interrupts on PIC1 & PIC2 are now masked\n");
}

void PIC_disable_irq(int irq_line_num){
	uint16_t port;
	uint8_t mask;
	uint8_t irq_line_bitmask;
	uint8_t irq_line_on_pic;

	if(!pics_initialized){
		printk_err("can't modify PICs without having initialized them first\n");
		return;
	}
	
	if(irq_line_num < 0 || irq_line_num > 15){
		printk_err("Attempted to disable irq_line %d (valid irqs are 0-7 for master pic and 8-15 for slave pic)\n", irq_line_num);
		return;
	}
	
	if(irq_line_num < 8){
		port = PIC1_DATA;
		irq_line_on_pic = irq_line_num;
	}
	else {
		port = PIC2_DATA;
		irq_line_on_pic = irq_line_num - 8;
	}

	irq_line_bitmask = (1 << irq_line_on_pic);
	
	mask = inb(port);

	if(mask & irq_line_bitmask){
		printk_warn("Attempted to disable irq %d but it was already disabled\n", irq_line_num);
		return;
	}
	
	printk_debug("mask before disable = 0x%x\n", mask);

	mask |= irq_line_bitmask;

	printk_debug("mask after disable = 0x%x\n", mask);
	
	outb(port, mask);
}

void PIC_enable_irq(int irq_line_num){
	uint16_t port;
	uint8_t mask;
	uint8_t irq_line_bitmask;
	uint8_t irq_line_on_pic;

	if(!pics_initialized){
		printk_err("can't modify PICs without having initialized them first\n");
		return;
	}
	
	if(irq_line_num < 0 || irq_line_num > 15){
		printk_err("Attempted to enable irq_line %d (valid irqs are 0-7 for master pic and 8-15 for slave pic)\n", irq_line_num);
		return;
	}
	
	if(irq_line_num < 8){
		port = PIC1_DATA;
		irq_line_on_pic = irq_line_num;
	}
	else {
		port = PIC2_DATA;
		irq_line_on_pic = irq_line_num - 8;
	}

	irq_line_bitmask = ~(1 << irq_line_on_pic);
	
	mask = inb(port);

	if(!(mask & ~irq_line_bitmask)){
		printk_warn("Attempted to enable irq %d but it was already enabled\n", irq_line_num);
		return;
	}

	mask &= irq_line_bitmask;
	
	outb(port, mask);
}

void PIC_end_of_interrupt(int irq){
	if(!pics_initialized){
		printk_err("can't modify PICs without having initialized them first\n");
		return;
	}
	
	if(PIC_irq_from_pic1(irq)){
		outb(PIC1_CMD, PIC_EOI);
		return;
	}
	if(PIC_irq_from_pic2(irq)){
		outb(PIC1_CMD, PIC_EOI);
		outb(PIC2_CMD, PIC_EOI);
		return;
	}
	printk_err("Erroneously attempted acknowledge irq %d (not from either PIC)\n", irq);
}

void PIC_print_all_irq_status(){
	uint8_t pic1_interrupts;
	uint8_t pic2_interrupts;
	
	if(!pics_initialized){
		printk_err("PICs not initialized yet!\n");
		return;
	}

	pic1_interrupts = inb(PIC1_DATA);
	pic2_interrupts = inb(PIC2_DATA);

	printk("--------------------------------------------------------\n");
	printk("pic1: 0x%x, pic2: 0x%x\n", pic1_interrupts, pic2_interrupts);
	printk("Master PIC irqs:\n");
	printk("System Timer    : %s\n", pic1_interrupts & (1 << SYSTEM_TIMER_IRQ) ? "disabled" : "enabled");
	printk("PS/2 Keyboard   : %s\n", pic1_interrupts & (1 << KEYBOARD_IRQ) ? "disabled" : "enabled");
	printk("Serial Com 2/4  : %s\n", pic1_interrupts & (1 << SERIAL_COM2_IRQ) ? "disabled" : "enabled");
	printk("Serial Com 1/3  : %s\n", pic1_interrupts & (1 << SERIAL_COM1_IRQ) ? "disabled" : "enabled");
	printk("Parallel Port 2 : %s\n", pic1_interrupts & (1 << PARALLEL_PORT2_IRQ) ? "disabled" : "enabled");
	printk("Floppy Disk     : %s\n", pic1_interrupts & (1 << FLOPPY_DISK_IRQ) ? "disabled" : "enabled");
	printk("Parallel Port 1 : %s\n", pic1_interrupts & (1 << PARALLEL_PORT1_IRQ) ? "disabled" : "enabled");

	printk("Slave PIC irqs:\n");
	printk("Real Time Clock : %s\n", pic2_interrupts & (1 << (CMOS_RTC_IRQ - 8)) ? "disabled" : "enabled");
	printk("Free or NIC 1   : %s\n", pic2_interrupts & (1 << (FREE_OR_NIC1_IRQ - 8)) ? "disabled" : "enabled");
	printk("Free or NIC 2   : %s\n", pic2_interrupts & (1 << (FREE_OR_NIC2_IRQ - 8)) ? "disabled" : "enabled");
	printk("Free or NIC 3   : %s\n", pic2_interrupts & (1 << (FREE_OR_NIC3_IRQ - 8)) ? "disabled" : "enabled");
	printk("PS/2 Mouse      : %s\n", pic2_interrupts & (1 << (MOUSE_IRQ - 8)) ? "disabled" : "enabled");
	printk("FPU             : %s\n", pic2_interrupts & (1 << (FPU_IRQ - 8)) ? "disabled" : "enabled");
	printk("Primary ATA     : %s\n", pic2_interrupts & (1 << (PRIMARY_ATA_CHANNEL_IRQ - 8)) ? "disabled" : "enabled");
	printk("Primary ATA     : %s\n", pic2_interrupts & (1 << (SECONDARY_ATA_CHANNEL_IRQ - 8)) ? "disabled" : "enabled");

	printk("--------------------------------------------------------\n");
}
