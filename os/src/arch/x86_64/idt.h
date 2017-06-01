#ifndef INTERRUPT
#define INTERRUPT

#define KEYBOARD_IRQ 1
#define SERIAL_COM2_IRQ 3
#define SERIAL_COM4_IRQ 3
#define SERIAL_COM1_IRQ 4
#define SERIAL_COM3_IRQ 4

#define NUM_PRIV_STACKS 3
#define NUM_INTERRUPT_STACKS 7

//exception irq vectors
#define DIVIDE_BY_ZERO_EX 0
#define DEBUG_EX 1
#define NON_MASKABLE_INT 2
#define BREAKPOINT_EX 3
#define OVERFLOW_EX 4
#define BOUNDED_RANGE_EX 5
#define INVALID_OPCODE_EX 6
#define DEVICE_NOT_AVAIL_EX 7
#define DOUBLE_FAULT_EX 8
#define INVALID_TSS_EX 10
#define SEG_NOT_PRES_EX 11
#define STACK_SEG_FAULT_EX 12
#define GENERAL_PROTECTION_FAULT_EX 13
#define PAGE_FAULT_EX 14
#define FLOATING_POINT_EX 16
#define ALIGNMENT_CHECK_EX 17
#define MACHINE_CHECK_EX 18
#define SIMD_FLOATING_POINT_EX 19
#define VIRTUALIZATION_EX 20
#define SECURITY_EX 30
#define SYSCALL_VECTOR 0x80

/**
  * SYSCALL numbers (defined by me)
  */
#define SYS_YIELD 24
#define SYS_EXIT 60
#define SYS_START 1
/*
 * Interrupt stack numbers
 * NOTE: unless #DF, #GP, or #PF will run on the default GENERAL_INTERRUPT_STACK
 */
#define GENERAL_INTERRUPT_STACK 0
#define DOUBLE_FAULT_STACK 1
#define GENERAL_PROTECTION_STACK 2
#define PAGE_FAULT_STACK 3
#define SYSCALL_STACK 4
typedef struct {
    uint32_t res1;
    uint64_t PST1;
    uint64_t PST2;
    uint64_t PST3;
    uint64_t res2;
    uint64_t IST1;
    uint64_t IST2;
    uint64_t IST3;
    uint64_t IST4;
    uint64_t IST5;
    uint64_t IST6;
    uint64_t IST7;
    uint64_t res3;
    uint16_t res4;
    uint16_t IOaddress;
    char IOMap[8];
}__attribute__((packed)) TaskStateSegment;

typedef struct {
    uint16_t seg_limit_1;
    uint16_t ptr_1;
    uint8_t ptr_2;
    uint16_t type:4;
    uint16_t must_be_0_1:1;
    uint16_t priv:2;
    uint16_t present:1;
    uint16_t seg_limit_2:4;
    uint16_t avl:1;
    uint16_t res1:2;
    uint16_t g:1;
    uint8_t ptr_3;
    uint32_t ptr_4;
    uint32_t must_be_0_2;
}__attribute__((packed)) TSSDesc;

// typedef struct{
//     uint32_t reserved_1;
//
//     //addresses of the stacks for the different privilege levels
//     uint64_t privilege_stack_table[NUM_PRIV_STACKS];
//
//     uint64_t reserved_2;
//
//     //interrupt stack table, contains addresses of the 7 different interrupt stacks
//     uint64_t interrupt_stack_table[NUM_INTERRUPT_STACKS];
//
//     uint64_t reserved_3;
//     uint16_t reserved_4;
//
//     // 16 bit offset to the I/O Permission Bit Map from the 64-bit TSS base
//     uint16_t iomap_base;
// }__attribute__((packed)) TaskStateSegment;

typedef struct {
    uint16_t firstTSSLimit:16;
    uint32_t firstTSSBase:24;
    uint8_t type:4; //must be 0b1001 for 64bit TSS
    uint8_t zero:1; //must be 0
    uint8_t privilege:2; //privilege level
    uint8_t present:1; //must be 1 for valid selector
    uint8_t secondTSSLimit:4; //we won't need this
    uint8_t available:1;
    uint8_t ignored:2;
    uint8_t granularity:1; //don't need
    uint8_t secondTSSBase;
    uint32_t lastTSSBase;
    uint32_t zero2:32;
}__attribute__((packed)) TSS_Segment;


typedef struct {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed)) gdt_tag;


void idt_init(void);
void IRQ_set_handler(int irq, void *handler);
void IRQ_set_mask(int IRQLine);
void IRQ_clear_mask(int IRQLine);

#endif
