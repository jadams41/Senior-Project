#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "serial.h"
#include "idt.h"
#include "memoryManager.h"
#include "utils.h"

extern void keyboard_handler(void);
extern void load_idt(unsigned long);
extern uint64_t gdt64[]; //pointer to the beginning of the old gdt
extern uint64_t new_GDT[]; //pointer to the beginning of the new gdt
gdt_tag gdt_desc;
extern void load_gdt(void);
extern void store_gdt(void);

extern uint64_t saved_cr2;
extern uint64_t saved_cr3;

gdt_tag gdtTagStruct;

extern void irq0_handler(void);
extern void irq1_handler(void);
extern void irq2_handler(void);
extern void irq3_handler(void);
extern void irq4_handler(void);
extern void irq5_handler(void);
extern void irq6_handler(void);
extern void irq7_handler(void);
extern void irq8_handler(void);
extern void irq9_handler(void);
extern void irq10_handler(void);
extern void irq11_handler(void);
extern void irq12_handler(void);
extern void irq13_handler(void);
extern void irq14_handler(void);
extern void irq15_handler(void);
extern void irq16_handler(void);
extern void irq17_handler(void);
extern void irq18_handler(void);
extern void irq19_handler(void);
extern void irq20_handler(void);
extern void irq21_handler(void);
extern void irq22_handler(void);
extern void irq23_handler(void);
extern void irq24_handler(void);
extern void irq25_handler(void);
extern void irq26_handler(void);
extern void irq27_handler(void);
extern void irq28_handler(void);
extern void irq29_handler(void);
extern void irq30_handler(void);
extern void irq31_handler(void);
extern void irq32_handler(void);
extern void irq33_handler(void);
extern void irq34_handler(void);
extern void irq35_handler(void);
extern void irq36_handler(void);
extern void irq37_handler(void);
extern void irq38_handler(void);
extern void irq39_handler(void);
extern void irq40_handler(void);
extern void irq41_handler(void);
extern void irq42_handler(void);
extern void irq43_handler(void);
extern void irq44_handler(void);
extern void irq45_handler(void);
extern void irq46_handler(void);
extern void irq47_handler(void);
extern void irq48_handler(void);
extern void irq49_handler(void);
extern void irq50_handler(void);
extern void irq51_handler(void);
extern void irq52_handler(void);
extern void irq53_handler(void);
extern void irq54_handler(void);
extern void irq55_handler(void);
extern void irq56_handler(void);
extern void irq57_handler(void);
extern void irq58_handler(void);
extern void irq59_handler(void);
extern void irq60_handler(void);
extern void irq61_handler(void);
extern void irq62_handler(void);
extern void irq63_handler(void);
extern void irq64_handler(void);
extern void irq65_handler(void);
extern void irq66_handler(void);
extern void irq67_handler(void);
extern void irq68_handler(void);
extern void irq69_handler(void);
extern void irq70_handler(void);
extern void irq71_handler(void);
extern void irq72_handler(void);
extern void irq73_handler(void);
extern void irq74_handler(void);
extern void irq75_handler(void);
extern void irq76_handler(void);
extern void irq77_handler(void);
extern void irq78_handler(void);
extern void irq79_handler(void);
extern void irq80_handler(void);
extern void irq81_handler(void);
extern void irq82_handler(void);
extern void irq83_handler(void);
extern void irq84_handler(void);
extern void irq85_handler(void);
extern void irq86_handler(void);
extern void irq87_handler(void);
extern void irq88_handler(void);
extern void irq89_handler(void);
extern void irq90_handler(void);
extern void irq91_handler(void);
extern void irq92_handler(void);
extern void irq93_handler(void);
extern void irq94_handler(void);
extern void irq95_handler(void);
extern void irq96_handler(void);
extern void irq97_handler(void);
extern void irq98_handler(void);
extern void irq99_handler(void);
extern void irq100_handler(void);
extern void irq101_handler(void);
extern void irq102_handler(void);
extern void irq103_handler(void);
extern void irq104_handler(void);
extern void irq105_handler(void);
extern void irq106_handler(void);
extern void irq107_handler(void);
extern void irq108_handler(void);
extern void irq109_handler(void);
extern void irq110_handler(void);
extern void irq111_handler(void);
extern void irq112_handler(void);
extern void irq113_handler(void);
extern void irq114_handler(void);
extern void irq115_handler(void);
extern void irq116_handler(void);
extern void irq117_handler(void);
extern void irq118_handler(void);
extern void irq119_handler(void);
extern void irq120_handler(void);
extern void irq121_handler(void);
extern void irq122_handler(void);
extern void irq123_handler(void);
extern void irq124_handler(void);
extern void irq125_handler(void);
extern void irq126_handler(void);
extern void irq127_handler(void);
extern void irq128_handler(void);
extern void irq129_handler(void);
extern void irq130_handler(void);
extern void irq131_handler(void);
extern void irq132_handler(void);
extern void irq133_handler(void);
extern void irq134_handler(void);
extern void irq135_handler(void);
extern void irq136_handler(void);
extern void irq137_handler(void);
extern void irq138_handler(void);
extern void irq139_handler(void);
extern void irq140_handler(void);
extern void irq141_handler(void);
extern void irq142_handler(void);
extern void irq143_handler(void);
extern void irq144_handler(void);
extern void irq145_handler(void);
extern void irq146_handler(void);
extern void irq147_handler(void);
extern void irq148_handler(void);
extern void irq149_handler(void);
extern void irq150_handler(void);
extern void irq151_handler(void);
extern void irq152_handler(void);
extern void irq153_handler(void);
extern void irq154_handler(void);
extern void irq155_handler(void);
extern void irq156_handler(void);
extern void irq157_handler(void);
extern void irq158_handler(void);
extern void irq159_handler(void);
extern void irq160_handler(void);
extern void irq161_handler(void);
extern void irq162_handler(void);
extern void irq163_handler(void);
extern void irq164_handler(void);
extern void irq165_handler(void);
extern void irq166_handler(void);
extern void irq167_handler(void);
extern void irq168_handler(void);
extern void irq169_handler(void);
extern void irq170_handler(void);
extern void irq171_handler(void);
extern void irq172_handler(void);
extern void irq173_handler(void);
extern void irq174_handler(void);
extern void irq175_handler(void);
extern void irq176_handler(void);
extern void irq177_handler(void);
extern void irq178_handler(void);
extern void irq179_handler(void);
extern void irq180_handler(void);
extern void irq181_handler(void);
extern void irq182_handler(void);
extern void irq183_handler(void);
extern void irq184_handler(void);
extern void irq185_handler(void);
extern void irq186_handler(void);
extern void irq187_handler(void);
extern void irq188_handler(void);
extern void irq189_handler(void);
extern void irq190_handler(void);
extern void irq191_handler(void);
extern void irq192_handler(void);
extern void irq193_handler(void);
extern void irq194_handler(void);
extern void irq195_handler(void);
extern void irq196_handler(void);
extern void irq197_handler(void);
extern void irq198_handler(void);
extern void irq199_handler(void);
extern void irq200_handler(void);
extern void irq201_handler(void);
extern void irq202_handler(void);
extern void irq203_handler(void);
extern void irq204_handler(void);
extern void irq205_handler(void);
extern void irq206_handler(void);
extern void irq207_handler(void);
extern void irq208_handler(void);
extern void irq209_handler(void);
extern void irq210_handler(void);
extern void irq211_handler(void);
extern void irq212_handler(void);
extern void irq213_handler(void);
extern void irq214_handler(void);
extern void irq215_handler(void);
extern void irq216_handler(void);
extern void irq217_handler(void);
extern void irq218_handler(void);
extern void irq219_handler(void);
extern void irq220_handler(void);
extern void irq221_handler(void);
extern void irq222_handler(void);
extern void irq223_handler(void);
extern void irq224_handler(void);
extern void irq225_handler(void);
extern void irq226_handler(void);
extern void irq227_handler(void);
extern void irq228_handler(void);
extern void irq229_handler(void);
extern void irq230_handler(void);
extern void irq231_handler(void);
extern void irq232_handler(void);
extern void irq233_handler(void);
extern void irq234_handler(void);
extern void irq235_handler(void);
extern void irq236_handler(void);
extern void irq237_handler(void);
extern void irq238_handler(void);
extern void irq239_handler(void);
extern void irq240_handler(void);
extern void irq241_handler(void);
extern void irq242_handler(void);
extern void irq243_handler(void);
extern void irq244_handler(void);
extern void irq245_handler(void);
extern void irq246_handler(void);
extern void irq247_handler(void);
extern void irq248_handler(void);
extern void irq249_handler(void);
extern void irq250_handler(void);
extern void irq251_handler(void);
extern void irq252_handler(void);
extern void irq253_handler(void);
extern void irq254_handler(void);
extern void irq255_handler(void);

#define IDT_SIZE 256

char ist_stack0[4096];
char ist_stack1[4096];
char ist_stack2[4096];
char ist_stack3[4096];
char ist_stack4[4096];
char ist_stack5[4096];
char ist_stack6[4096];

uint64_t ist_stack0_top = (uint64_t) ist_stack0 + 4096;
uint64_t ist_stack1_top = (uint64_t) ist_stack1 + 4096;
uint64_t ist_stack2_top = (uint64_t) ist_stack2 + 4096;
uint64_t ist_stack3_top = (uint64_t) ist_stack3 + 4096;
uint64_t ist_stack4_top = (uint64_t) ist_stack4 + 4096;
uint64_t ist_stack5_top = (uint64_t) ist_stack5 + 4096;
uint64_t ist_stack6_top = (uint64_t) ist_stack6 + 4096;

TaskStateSegment tss;
TSSDesc tssdesc;
TSS_Segment tss_seg;

//todo remove all references to these and replace with the functions in
//utils.c and ensure that the functionality is the same
//note: port and val may be switched
void inline outby(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

//
uint8_t inline inby(uint16_t port) {
    uint8_t val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

//todo move this structure to the header file
typedef struct {
    uint16_t funct_ptr_1;
    uint16_t gdt_select;
    uint16_t ist:3;
    uint16_t res:5;
    uint16_t type:4;
    uint16_t must_be_0:1;
    uint16_t priv_lvl:2;
    uint16_t present:1;
    uint16_t funct_ptr_2;
    uint32_t funct_ptr_3;
    uint32_t reserved;
}__attribute__((packed)) IDT_Entry;

IDT_Entry idt[IDT_SIZE];

void (*ISRs[IDT_SIZE])(void) = {
	irq0_handler,
	irq1_handler,
	irq2_handler,
	irq3_handler,
	irq4_handler,
	irq5_handler,
	irq6_handler,
	irq7_handler,
	irq8_handler,
	irq9_handler,
	irq10_handler,
	irq11_handler,
	irq12_handler,
	irq13_handler,
	irq14_handler,
	irq15_handler,
	irq16_handler,
	irq17_handler,
	irq18_handler,
	irq19_handler,
	irq20_handler,
	irq21_handler,
	irq22_handler,
	irq23_handler,
	irq24_handler,
	irq25_handler,
	irq26_handler,
	irq27_handler,
	irq28_handler,
	irq29_handler,
	irq30_handler,
	irq31_handler,
	irq32_handler,
	irq33_handler,
	irq34_handler,
	irq35_handler,
	irq36_handler,
	irq37_handler,
	irq38_handler,
	irq39_handler,
	irq40_handler,
	irq41_handler,
	irq42_handler,
	irq43_handler,
	irq44_handler,
	irq45_handler,
	irq46_handler,
	irq47_handler,
	irq48_handler,
	irq49_handler,
	irq50_handler,
	irq51_handler,
	irq52_handler,
	irq53_handler,
	irq54_handler,
	irq55_handler,
	irq56_handler,
	irq57_handler,
	irq58_handler,
	irq59_handler,
	irq60_handler,
	irq61_handler,
	irq62_handler,
	irq63_handler,
	irq64_handler,
	irq65_handler,
	irq66_handler,
	irq67_handler,
	irq68_handler,
	irq69_handler,
	irq70_handler,
	irq71_handler,
	irq72_handler,
	irq73_handler,
	irq74_handler,
	irq75_handler,
	irq76_handler,
	irq77_handler,
	irq78_handler,
	irq79_handler,
	irq80_handler,
	irq81_handler,
	irq82_handler,
	irq83_handler,
	irq84_handler,
	irq85_handler,
	irq86_handler,
	irq87_handler,
	irq88_handler,
	irq89_handler,
	irq90_handler,
	irq91_handler,
	irq92_handler,
	irq93_handler,
	irq94_handler,
	irq95_handler,
	irq96_handler,
	irq97_handler,
	irq98_handler,
	irq99_handler,
	irq100_handler,
	irq101_handler,
	irq102_handler,
	irq103_handler,
	irq104_handler,
	irq105_handler,
	irq106_handler,
	irq107_handler,
	irq108_handler,
	irq109_handler,
	irq110_handler,
	irq111_handler,
	irq112_handler,
	irq113_handler,
	irq114_handler,
	irq115_handler,
	irq116_handler,
	irq117_handler,
	irq118_handler,
	irq119_handler,
	irq120_handler,
	irq121_handler,
	irq122_handler,
	irq123_handler,
	irq124_handler,
	irq125_handler,
	irq126_handler,
	irq127_handler,
	irq128_handler,
	irq129_handler,
	irq130_handler,
	irq131_handler,
	irq132_handler,
	irq133_handler,
	irq134_handler,
	irq135_handler,
	irq136_handler,
	irq137_handler,
	irq138_handler,
	irq139_handler,
	irq140_handler,
	irq141_handler,
	irq142_handler,
	irq143_handler,
	irq144_handler,
	irq145_handler,
	irq146_handler,
	irq147_handler,
	irq148_handler,
	irq149_handler,
	irq150_handler,
	irq151_handler,
	irq152_handler,
	irq153_handler,
	irq154_handler,
	irq155_handler,
	irq156_handler,
	irq157_handler,
	irq158_handler,
	irq159_handler,
	irq160_handler,
	irq161_handler,
	irq162_handler,
	irq163_handler,
	irq164_handler,
	irq165_handler,
	irq166_handler,
	irq167_handler,
	irq168_handler,
	irq169_handler,
	irq170_handler,
	irq171_handler,
	irq172_handler,
	irq173_handler,
	irq174_handler,
	irq175_handler,
	irq176_handler,
	irq177_handler,
	irq178_handler,
	irq179_handler,
	irq180_handler,
	irq181_handler,
	irq182_handler,
	irq183_handler,
	irq184_handler,
	irq185_handler,
	irq186_handler,
	irq187_handler,
	irq188_handler,
	irq189_handler,
	irq190_handler,
	irq191_handler,
	irq192_handler,
	irq193_handler,
	irq194_handler,
	irq195_handler,
	irq196_handler,
	irq197_handler,
	irq198_handler,
	irq199_handler,
	irq200_handler,
	irq201_handler,
	irq202_handler,
	irq203_handler,
	irq204_handler,
	irq205_handler,
	irq206_handler,
	irq207_handler,
	irq208_handler,
	irq209_handler,
	irq210_handler,
	irq211_handler,
	irq212_handler,
	irq213_handler,
	irq214_handler,
	irq215_handler,
	irq216_handler,
	irq217_handler,
	irq218_handler,
	irq219_handler,
	irq220_handler,
	irq221_handler,
	irq222_handler,
	irq223_handler,
	irq224_handler,
	irq225_handler,
	irq226_handler,
	irq227_handler,
	irq228_handler,
	irq229_handler,
	irq230_handler,
	irq231_handler,
	irq232_handler,
	irq233_handler,
	irq234_handler,
	irq235_handler,
	irq236_handler,
	irq237_handler,
	irq238_handler,
	irq239_handler,
	irq240_handler,
	irq241_handler,
	irq242_handler,
	irq243_handler,
	irq244_handler,
	irq245_handler,
	irq246_handler,
	irq247_handler,
	irq248_handler,
	irq249_handler,
	irq250_handler,
	irq251_handler,
	irq252_handler,
	irq253_handler,
	irq254_handler,
	irq255_handler
};

//array of the addresses for the c_isrs, will be initialized to zero
//however, can be set with IRQ_set_handler
void (*c_ISRs[IDT_SIZE])(int, int);

//keyboard isr
void keyboard_isr(int irq, int err){
    enableSerialPrinting();
    pollInputBuffer();
    char val = inby(0x60);
    keyboard_handler_main((char)val);
    disableSerialPrinting();
}

//handler for if an isr faults
void double_fault_handler(int irq, int err){
    printk_err("double fault\n");
    asm("hlt");
}

//serial isr
void serial_isr(int irq, int err){
    //indirect way to call consume byte
    SER_write("",0);
}

void general_protection_fault_handler(int irq, int err){
    printk_err("general protection fault\n");
    asm("hlt");
}

void page_fault_handler(int irq, int err){
    //saved cr2 is the faulting address that the program tried to acces
    //saved cr3 is the address of the base of the page (level 4) table
    uint64_t p4_index, p3_index, p2_index, p1_index, grabber = 0b111111111; //grabber grabs the 9 bit index into the given table

    grabber <<= 12;
    p1_index = (saved_cr2 & grabber) >> 12;
    grabber <<= 9;
    p2_index = (saved_cr2 & grabber) >> 21;
    grabber <<= 9;
    p3_index = (saved_cr2 & grabber) >> 30;
    grabber <<= 9;
    p4_index = (saved_cr2 & grabber) >> 39;

    //grab the index of the page 3 table pointed to by p4 index (from cr2) into the saved_cr3 (p4_table)
    uint64_t *table_ptr = (uint64_t*)((uint64_t*)saved_cr3)[p4_index];
    if(!entry_present((uint64_t)table_ptr)) goto page_table_error;
    table_ptr = strip_present_bits(table_ptr);

    table_ptr = (uint64_t*)table_ptr[p3_index];
    if(!entry_present((uint64_t)table_ptr)) goto page_table_error;
    table_ptr = strip_present_bits(table_ptr);

    table_ptr = (uint64_t*)table_ptr[p2_index];
    if(!entry_present((uint64_t)table_ptr)) goto page_table_error;
    table_ptr = strip_present_bits(table_ptr);

    if((table_ptr[p1_index] & 0b111000000000) == 0){
        page_table_error:
        printk_err("page fault on non-on-demand page\n");
        printk("   Fault details: cr2=0x%lx cr3=0x%lx", saved_cr2, saved_cr3);
        asm("hlt");
    }
    else {
        void *newPage = MMU_pf_alloc();
        if(newPage == 0){
            //error allocating a physical page
            printk_err("couldn't get a physical page for the virtual one, halting because we can't resolve the fault\n");
            asm("hlt");
        }
        // printk("successfully grabbed a physical page (%lx) on demand!\n", newPage);
        uint64_t pg = (uint64_t)newPage;
        // zero_out_page(newPage);
        pg |= 0b11; //set available and write
        table_ptr[p1_index] = pg;
    }
}

void IRQ_set_handler(int irq, void *handler){
    c_ISRs[irq] = handler;
}

//initialize all things required for having separate stacks for certain faults
static void __attribute__((unused)) ist_init(){
    //initialize the TSS
    tss.IST1 = (uint64_t)ist_stack0 + 4096;
    tss.IST2 = (uint64_t)ist_stack1 + 4096;
    tss.IST3 = (uint64_t)ist_stack2 + 4096;
    tss.IST4 = (uint64_t)ist_stack3 + 4096;
    tss.IST5 = (uint64_t)ist_stack4 + 4096;
    tss.IST6 = (uint64_t)ist_stack5 + 4096;
    tss.IST7 = (uint64_t)ist_stack6 + 4096;
    int i;
    for(i=0; i<7; i++)
        tss.IOMap[i] = 0;
    tss.IOMap[7] = -1;
    tss.IOaddress = 104;

    tss.res1 = 0;
    tss.PST1 = 0;
    tss.PST2 = 0;
    tss.PST3 = 0;
    tss.res2 = 0;
    tss.res3 = 0;
    tss.res4 = 0;

    //initialize TSS segment
    uint64_t tss_ptr = (uint64_t)&tss;

    tss_seg.firstTSSLimit = (uint16_t) 112;//sizeof(TaskStateSegment) - 1; //todo this might be wrong
    tss_seg.firstTSSBase = tss_ptr & 0x0000000000FFFFFF;
    tss_seg.secondTSSBase = (tss_ptr & 0x00000000FF000000) >> 24;
    tss_seg.lastTSSBase = (tss_ptr & 0xFFFFFFFF00000000) >> 32;
    tss_seg.type = 0b1001; //0b1001
    tss_seg.zero = 0;
    tss_seg.privilege = 0;
    tss_seg.present = 1;
    tss_seg.secondTSSLimit = 0;
    tss_seg.ignored = 0;
    tss_seg.available = 0;
    tss_seg.granularity = 0;
    tss_seg.zero2 = 0;

    uint64_t *tss_seg_pointer = (uint64_t*)&tss_seg;
    uint16_t selector = 0x10;
    gdt64[2] = tss_seg_pointer[0];
    gdt64[3] = tss_seg_pointer[1];
    asm( "ltr %0" : : "m"(selector) );
}

void idt_init(void){
    //fill the IDT with entries that contain relevant information and point to
    //the respective assembly routines
    int i;
    uint16_t istNum;
	for(i = 0; i < IDT_SIZE; i++){
	    IDT_Entry *entry = idt + i;
	    entry->funct_ptr_1 = (uint64_t) ISRs[i] & 0x000000000000FFFF;
	    entry->gdt_select = 0x08;
        //conditionally set the ist
        istNum = GENERAL_INTERRUPT_STACK;

        if(i == GENERAL_PROTECTION_FAULT_EX){
            istNum = GENERAL_PROTECTION_FAULT_EX;
        }
        else if (i == DOUBLE_FAULT_EX){
            istNum = DOUBLE_FAULT_STACK;
        }
        else if(i == PAGE_FAULT_EX){
            istNum = PAGE_FAULT_STACK;
        }
        //this is for testing, mapping the keyboard interrupt to another stack
        else if(i == 33){
            istNum = 4;
        }

        entry->ist = istNum;
	    entry->res = 0;
	    entry->type = 0xF;
	    entry->must_be_0 = 0;
	    entry->priv_lvl = 0;
	    entry->present = 1;
	    entry->funct_ptr_2 = ((uint64_t) ISRs[i] & 0x00000000FFFF0000) >> 16;
	    entry->funct_ptr_3 = ((uint64_t) ISRs[i] & 0xFFFFFFFF00000000) >> 32;
	    entry->reserved = 0;
    }

    //zero out all of the c_ISRs
    for(i = 0; i < IDT_SIZE; i++){
        IRQ_set_handler(i, (void*)0);
    }

	/* here are the ports for the 2 PICs
	 *
	 * PIC1 cmd: 0x20 data: 0x21
	 * PIC2 cmd: 0xA0 data: 0xA1
	 */

	/* ICW1 - begin initialization of the PIC */
	outby(0x20, 0x11);
	outby(0xA0, 0x11);

	/* ICW2 - remap the offset address of IDT */
	/*
	 * In x86 protected mode, we have to remap the PICs beyond 0x20 because
	 * Intel reserved the first 32 interrupts for cpu exceptions
	 */
	outby(0x21, 0x20);
	//second PIC starts 8 bytes after the first because each has 8 lines
	outby(0xA1, 0x28);

	/* ICW3 - setup cascading */
	outby(0x21, 0x00);
	outby(0xA1, 0x00);

	/* ICW4 - environment info */
	outby(0x21, 0x01);
	outby(0xA1, 0x01);

	//mask all interrupts
	outby(0x21, 0xff);
	outby(0xA1, 0xff);

    //todo, move this struct to the header file
    struct {
        uint16_t length;
        void*    base;
	} __attribute__((packed)) IDTR = { sizeof(IDT_Entry) * 256, idt };

    //todo, move this to a macro in utils
	asm ( "lidt %0" : : "m"(IDTR) );

    //todo, move the isrs out of this file
    //and then move these function calls somewhere else
    IRQ_set_handler(33, keyboard_isr);
    IRQ_set_handler(36, serial_isr);
    IRQ_set_handler(8, double_fault_handler);
    IRQ_set_handler(GENERAL_PROTECTION_FAULT_EX, general_protection_fault_handler);
    IRQ_set_handler(PAGE_FAULT_EX, page_fault_handler);

    ist_init();
}

void IRQ_set_mask(int IRQLine){
    uint16_t port;
    uint8_t value;

    if(IRQLine < 8){
        port = 0x21;
    }
    else {
        port = 0xA1;
        IRQLine -= 8;
    }
    value = inby(port) | (1 << IRQLine);
    outby(port, value);
}

void IRQ_clear_mask(int IRQLine){
    uint16_t port;
    uint8_t value;

    if(IRQLine < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        IRQLine -= 8;
    }
    value = inby(port) & ~(1 << IRQLine);
    outby(port, value);
}

void generic_c_isr(int irq, int err){
    disableSerialPrinting();
    void (*loadedHandler)(int, int) = c_ISRs[irq];

    if(loadedHandler == 0){
        //interrupt was triggered without a loaded isr in the IDT
        printk_err("received interrupt on IRQ %d, however there was no ISR installed\n", irq);
    }
    else {
        //ISR has been set and will be called
        loadedHandler(irq, err);
    }

    if(irq > 8){
		outby(0xA0, 0x20);
	}
	outby(0x20, 0x20);
    enableSerialPrinting();
}
