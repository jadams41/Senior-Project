#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"

extern void keyboard_handler(void);
extern void load_idt(unsigned long);

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

void inline outby(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inline inby(uint16_t port) {
    uint8_t val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

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

IDT_Entry IDT[IDT_SIZE];
void c_isr0(int irq, int err){
    return;
}
void c_isr1(int irq, int err){
    return;
}
void c_isr2(int irq, int err){
    return;
}
void c_isr3(int irq, int err){
    return;
}
void c_isr4(int irq, int err){
    return;
}
void c_isr5(int irq, int err){
    return;
}
void c_isr6(int irq, int err){
    return;
}
void c_isr7(int irq, int err){
    return;
}
void c_isr8(int irq, int err){
    return;
}
void c_isr9(int irq, int err){
    return;
}
void c_isr10(int irq, int err){
    return;
}
void c_isr11(int irq, int err){
    return;
}
void c_isr12(int irq, int err){
    return;
}
void c_isr13(int irq, int err){
    return;
}
void c_isr14(int irq, int err){
    return;
}
void c_isr15(int irq, int err){
    return;
}
void c_isr16(int irq, int err){
    return;
}
void c_isr17(int irq, int err){
    return;
}
void c_isr18(int irq, int err){
    return;
}
void c_isr19(int irq, int err){
    return;
}
void c_isr20(int irq, int err){
    return;
}
void c_isr21(int irq, int err){
    return;
}
void c_isr22(int irq, int err){
    return;
}
void c_isr23(int irq, int err){
    return;
}
void c_isr24(int irq, int err){
    return;
}
void c_isr25(int irq, int err){
    return;
}
void c_isr26(int irq, int err){
    return;
}
void c_isr27(int irq, int err){
    return;
}
void c_isr28(int irq, int err){
    return;
}
void c_isr29(int irq, int err){
    return;
}
void c_isr30(int irq, int err){
    return;
}
void c_isr31(int irq, int err){
    return;
}
void c_isr32(int irq, int err){
	return;
}
void c_isr33(int irq, int err){
	pollInputBuffer();
    char val = inby(0x60);
    keyboard_handler_main((char)val);
}
void c_isr34(int irq, int err){
    return;
}
void c_isr35(int irq, int err){
    return;
}
void c_isr36(int irq, int err){
    return;
}
void c_isr37(int irq, int err){
    return;
}
void c_isr38(int irq, int err){
    return;
}
void c_isr39(int irq, int err){
    return;
}
void c_isr40(int irq, int err){
    return;
}
void c_isr41(int irq, int err){
    return;
}
void c_isr42(int irq, int err){
    return;
}
void c_isr43(int irq, int err){
    return;
}
void c_isr44(int irq, int err){
    return;
}
void c_isr45(int irq, int err){
    return;
}
void c_isr46(int irq, int err){
    return;
}
void c_isr47(int irq, int err){
    return;
}
void c_isr48(int irq, int err){
    return;
}
void c_isr49(int irq, int err){
    return;
}
void c_isr50(int irq, int err){
    return;
}
void c_isr51(int irq, int err){
    return;
}
void c_isr52(int irq, int err){
    return;
}
void c_isr53(int irq, int err){
    return;
}
void c_isr54(int irq, int err){
    return;
}
void c_isr55(int irq, int err){
    return;
}
void c_isr56(int irq, int err){
    return;
}
void c_isr57(int irq, int err){
    return;
}
void c_isr58(int irq, int err){
    return;
}
void c_isr59(int irq, int err){
    return;
}
void c_isr60(int irq, int err){
    return;
}
void c_isr61(int irq, int err){
    return;
}
void c_isr62(int irq, int err){
    return;
}
void c_isr63(int irq, int err){
    return;
}
void c_isr64(int irq, int err){
    return;
}
void c_isr65(int irq, int err){
    return;
}
void c_isr66(int irq, int err){
    return;
}
void c_isr67(int irq, int err){
    return;
}
void c_isr68(int irq, int err){
    return;
}
void c_isr69(int irq, int err){
    return;
}
void c_isr70(int irq, int err){
    return;
}
void c_isr71(int irq, int err){
    return;
}
void c_isr72(int irq, int err){
    return;
}
void c_isr73(int irq, int err){
    return;
}
void c_isr74(int irq, int err){
    return;
}
void c_isr75(int irq, int err){
    return;
}
void c_isr76(int irq, int err){
    return;
}
void c_isr77(int irq, int err){
    return;
}
void c_isr78(int irq, int err){
    return;
}
void c_isr79(int irq, int err){
    return;
}
void c_isr80(int irq, int err){
    return;
}
void c_isr81(int irq, int err){
    return;
}
void c_isr82(int irq, int err){
    return;
}
void c_isr83(int irq, int err){
    return;
}
void c_isr84(int irq, int err){
    return;
}
void c_isr85(int irq, int err){
    return;
}
void c_isr86(int irq, int err){
    return;
}
void c_isr87(int irq, int err){
    return;
}
void c_isr88(int irq, int err){
    return;
}
void c_isr89(int irq, int err){
    return;
}
void c_isr90(int irq, int err){
    return;
}
void c_isr91(int irq, int err){
    return;
}
void c_isr92(int irq, int err){
    return;
}
void c_isr93(int irq, int err){
    return;
}
void c_isr94(int irq, int err){
    return;
}
void c_isr95(int irq, int err){
    return;
}
void c_isr96(int irq, int err){
    return;
}
void c_isr97(int irq, int err){
    return;
}
void c_isr98(int irq, int err){
    return;
}
void c_isr99(int irq, int err){
    return;
}
void c_isr100(int irq, int err){
    return;
}
void c_isr101(int irq, int err){
    return;
}
void c_isr102(int irq, int err){
    return;
}
void c_isr103(int irq, int err){
    return;
}
void c_isr104(int irq, int err){
    return;
}
void c_isr105(int irq, int err){
    return;
}
void c_isr106(int irq, int err){
    return;
}
void c_isr107(int irq, int err){
    return;
}
void c_isr108(int irq, int err){
    return;
}
void c_isr109(int irq, int err){
    return;
}
void c_isr110(int irq, int err){
    return;
}
void c_isr111(int irq, int err){
    return;
}
void c_isr112(int irq, int err){
    return;
}
void c_isr113(int irq, int err){
    return;
}
void c_isr114(int irq, int err){
    return;
}
void c_isr115(int irq, int err){
    return;
}
void c_isr116(int irq, int err){
    return;
}
void c_isr117(int irq, int err){
    return;
}
void c_isr118(int irq, int err){
    return;
}
void c_isr119(int irq, int err){
    return;
}
void c_isr120(int irq, int err){
    return;
}
void c_isr121(int irq, int err){
    return;
}
void c_isr122(int irq, int err){
    return;
}
void c_isr123(int irq, int err){
    return;
}
void c_isr124(int irq, int err){
    return;
}
void c_isr125(int irq, int err){
    return;
}
void c_isr126(int irq, int err){
    return;
}
void c_isr127(int irq, int err){
    return;
}
void c_isr128(int irq, int err){
    return;
}
void c_isr129(int irq, int err){
    return;
}
void c_isr130(int irq, int err){
    return;
}
void c_isr131(int irq, int err){
    return;
}
void c_isr132(int irq, int err){
    return;
}
void c_isr133(int irq, int err){
    return;
}
void c_isr134(int irq, int err){
    return;
}
void c_isr135(int irq, int err){
    return;
}
void c_isr136(int irq, int err){
    return;
}
void c_isr137(int irq, int err){
    return;
}
void c_isr138(int irq, int err){
    return;
}
void c_isr139(int irq, int err){
    return;
}
void c_isr140(int irq, int err){
    return;
}
void c_isr141(int irq, int err){
    return;
}
void c_isr142(int irq, int err){
    return;
}
void c_isr143(int irq, int err){
    return;
}
void c_isr144(int irq, int err){
    return;
}
void c_isr145(int irq, int err){
    return;
}
void c_isr146(int irq, int err){
    return;
}
void c_isr147(int irq, int err){
    return;
}
void c_isr148(int irq, int err){
    return;
}
void c_isr149(int irq, int err){
    return;
}
void c_isr150(int irq, int err){
    return;
}
void c_isr151(int irq, int err){
    return;
}
void c_isr152(int irq, int err){
    return;
}
void c_isr153(int irq, int err){
    return;
}
void c_isr154(int irq, int err){
    return;
}
void c_isr155(int irq, int err){
    return;
}
void c_isr156(int irq, int err){
    return;
}
void c_isr157(int irq, int err){
    return;
}
void c_isr158(int irq, int err){
    return;
}
void c_isr159(int irq, int err){
    return;
}
void c_isr160(int irq, int err){
    return;
}
void c_isr161(int irq, int err){
    return;
}
void c_isr162(int irq, int err){
    return;
}
void c_isr163(int irq, int err){
    return;
}
void c_isr164(int irq, int err){
    return;
}
void c_isr165(int irq, int err){
    return;
}
void c_isr166(int irq, int err){
    return;
}
void c_isr167(int irq, int err){
    return;
}
void c_isr168(int irq, int err){
    return;
}
void c_isr169(int irq, int err){
    return;
}
void c_isr170(int irq, int err){
    return;
}
void c_isr171(int irq, int err){
    return;
}
void c_isr172(int irq, int err){
    return;
}
void c_isr173(int irq, int err){
    return;
}
void c_isr174(int irq, int err){
    return;
}
void c_isr175(int irq, int err){
    return;
}
void c_isr176(int irq, int err){
    return;
}
void c_isr177(int irq, int err){
    return;
}
void c_isr178(int irq, int err){
    return;
}
void c_isr179(int irq, int err){
    return;
}
void c_isr180(int irq, int err){
    return;
}
void c_isr181(int irq, int err){
    return;
}
void c_isr182(int irq, int err){
    return;
}
void c_isr183(int irq, int err){
    return;
}
void c_isr184(int irq, int err){
    return;
}
void c_isr185(int irq, int err){
    return;
}
void c_isr186(int irq, int err){
    return;
}
void c_isr187(int irq, int err){
    return;
}
void c_isr188(int irq, int err){
    return;
}
void c_isr189(int irq, int err){
    return;
}
void c_isr190(int irq, int err){
    return;
}
void c_isr191(int irq, int err){
    return;
}
void c_isr192(int irq, int err){
    return;
}
void c_isr193(int irq, int err){
    return;
}
void c_isr194(int irq, int err){
    return;
}
void c_isr195(int irq, int err){
    return;
}
void c_isr196(int irq, int err){
    return;
}
void c_isr197(int irq, int err){
    return;
}
void c_isr198(int irq, int err){
    return;
}
void c_isr199(int irq, int err){
    return;
}
void c_isr200(int irq, int err){
    return;
}
void c_isr201(int irq, int err){
    return;
}
void c_isr202(int irq, int err){
    return;
}
void c_isr203(int irq, int err){
    return;
}
void c_isr204(int irq, int err){
    return;
}
void c_isr205(int irq, int err){
    return;
}
void c_isr206(int irq, int err){
    return;
}
void c_isr207(int irq, int err){
    return;
}
void c_isr208(int irq, int err){
    return;
}
void c_isr209(int irq, int err){
    return;
}
void c_isr210(int irq, int err){
    return;
}
void c_isr211(int irq, int err){
    return;
}
void c_isr212(int irq, int err){
    return;
}
void c_isr213(int irq, int err){
    return;
}
void c_isr214(int irq, int err){
    return;
}
void c_isr215(int irq, int err){
    return;
}
void c_isr216(int irq, int err){
    return;
}
void c_isr217(int irq, int err){
    return;
}
void c_isr218(int irq, int err){
    return;
}
void c_isr219(int irq, int err){
    return;
}
void c_isr220(int irq, int err){
    return;
}
void c_isr221(int irq, int err){
    return;
}
void c_isr222(int irq, int err){
    return;
}
void c_isr223(int irq, int err){
    return;
}
void c_isr224(int irq, int err){
    return;
}
void c_isr225(int irq, int err){
    return;
}
void c_isr226(int irq, int err){
    return;
}
void c_isr227(int irq, int err){
    return;
}
void c_isr228(int irq, int err){
    return;
}
void c_isr229(int irq, int err){
    return;
}
void c_isr230(int irq, int err){
    return;
}
void c_isr231(int irq, int err){
    return;
}
void c_isr232(int irq, int err){
    return;
}
void c_isr233(int irq, int err){
    return;
}
void c_isr234(int irq, int err){
    return;
}
void c_isr235(int irq, int err){
    return;
}
void c_isr236(int irq, int err){
    return;
}
void c_isr237(int irq, int err){
    return;
}
void c_isr238(int irq, int err){
    return;
}
void c_isr239(int irq, int err){
    return;
}
void c_isr240(int irq, int err){
    return;
}
void c_isr241(int irq, int err){
    return;
}
void c_isr242(int irq, int err){
    return;
}
void c_isr243(int irq, int err){
    return;
}
void c_isr244(int irq, int err){
    return;
}
void c_isr245(int irq, int err){
    return;
}
void c_isr246(int irq, int err){
    return;
}
void c_isr247(int irq, int err){
    return;
}
void c_isr248(int irq, int err){
    return;
}
void c_isr249(int irq, int err){
    return;
}
void c_isr250(int irq, int err){
    return;
}
void c_isr251(int irq, int err){
    return;
}
void c_isr252(int irq, int err){
    return;
}
void c_isr253(int irq, int err){
    return;
}
void c_isr254(int irq, int err){
    return;
}
void c_isr255(int irq, int err){
    return;
}

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

void (*c_ISRs[IDT_SIZE])(int, int) = {
	c_isr0,
	c_isr1,
	c_isr2,
	c_isr3,
	c_isr4,
	c_isr5,
	c_isr6,
	c_isr7,
	c_isr8,
	c_isr9,
	c_isr10,
	c_isr11,
	c_isr12,
	c_isr13,
	c_isr14,
	c_isr15,
	c_isr16,
	c_isr17,
	c_isr18,
	c_isr19,
	c_isr20,
	c_isr21,
	c_isr22,
	c_isr23,
	c_isr24,
	c_isr25,
	c_isr26,
	c_isr27,
	c_isr28,
	c_isr29,
	c_isr30,
	c_isr31,
	c_isr32,
	c_isr33,
	c_isr34,
	c_isr35,
	c_isr36,
	c_isr37,
	c_isr38,
	c_isr39,
	c_isr40,
	c_isr41,
	c_isr42,
	c_isr43,
	c_isr44,
	c_isr45,
	c_isr46,
	c_isr47,
	c_isr48,
	c_isr49,
	c_isr50,
	c_isr51,
	c_isr52,
	c_isr53,
	c_isr54,
	c_isr55,
	c_isr56,
	c_isr57,
	c_isr58,
	c_isr59,
	c_isr60,
	c_isr61,
	c_isr62,
	c_isr63,
	c_isr64,
	c_isr65,
	c_isr66,
	c_isr67,
	c_isr68,
	c_isr69,
	c_isr70,
	c_isr71,
	c_isr72,
	c_isr73,
	c_isr74,
	c_isr75,
	c_isr76,
	c_isr77,
	c_isr78,
	c_isr79,
	c_isr80,
	c_isr81,
	c_isr82,
	c_isr83,
	c_isr84,
	c_isr85,
	c_isr86,
	c_isr87,
	c_isr88,
	c_isr89,
	c_isr90,
	c_isr91,
	c_isr92,
	c_isr93,
	c_isr94,
	c_isr95,
	c_isr96,
	c_isr97,
	c_isr98,
	c_isr99,
	c_isr100,
	c_isr101,
	c_isr102,
	c_isr103,
	c_isr104,
	c_isr105,
	c_isr106,
	c_isr107,
	c_isr108,
	c_isr109,
	c_isr110,
	c_isr111,
	c_isr112,
	c_isr113,
	c_isr114,
	c_isr115,
	c_isr116,
	c_isr117,
	c_isr118,
	c_isr119,
	c_isr120,
	c_isr121,
	c_isr122,
	c_isr123,
	c_isr124,
	c_isr125,
	c_isr126,
	c_isr127,
	c_isr128,
	c_isr129,
	c_isr130,
	c_isr131,
	c_isr132,
	c_isr133,
	c_isr134,
	c_isr135,
	c_isr136,
	c_isr137,
	c_isr138,
	c_isr139,
	c_isr140,
	c_isr141,
	c_isr142,
	c_isr143,
	c_isr144,
	c_isr145,
	c_isr146,
	c_isr147,
	c_isr148,
	c_isr149,
	c_isr150,
	c_isr151,
	c_isr152,
	c_isr153,
	c_isr154,
	c_isr155,
	c_isr156,
	c_isr157,
	c_isr158,
	c_isr159,
	c_isr160,
	c_isr161,
	c_isr162,
	c_isr163,
	c_isr164,
	c_isr165,
	c_isr166,
	c_isr167,
	c_isr168,
	c_isr169,
	c_isr170,
	c_isr171,
	c_isr172,
	c_isr173,
	c_isr174,
	c_isr175,
	c_isr176,
	c_isr177,
	c_isr178,
	c_isr179,
	c_isr180,
	c_isr181,
	c_isr182,
	c_isr183,
	c_isr184,
	c_isr185,
	c_isr186,
	c_isr187,
	c_isr188,
	c_isr189,
	c_isr190,
	c_isr191,
	c_isr192,
	c_isr193,
	c_isr194,
	c_isr195,
	c_isr196,
	c_isr197,
	c_isr198,
	c_isr199,
	c_isr200,
	c_isr201,
	c_isr202,
	c_isr203,
	c_isr204,
	c_isr205,
	c_isr206,
	c_isr207,
	c_isr208,
	c_isr209,
	c_isr210,
	c_isr211,
	c_isr212,
	c_isr213,
	c_isr214,
	c_isr215,
	c_isr216,
	c_isr217,
	c_isr218,
	c_isr219,
	c_isr220,
	c_isr221,
	c_isr222,
	c_isr223,
	c_isr224,
	c_isr225,
	c_isr226,
	c_isr227,
	c_isr228,
	c_isr229,
	c_isr230,
	c_isr231,
	c_isr232,
	c_isr233,
	c_isr234,
	c_isr235,
	c_isr236,
	c_isr237,
	c_isr238,
	c_isr239,
	c_isr240,
	c_isr241,
	c_isr242,
	c_isr243,
	c_isr244,
	c_isr245,
	c_isr246,
	c_isr247,
	c_isr248,
	c_isr249,
	c_isr250,
	c_isr251,
	c_isr252,
	c_isr253,
	c_isr254,
	c_isr255
};

void idt_init(void)
{
	unsigned long keyboard_address;
	//unsigned long idt_address;
	//unsigned long idt_ptr[2];

 
	//set the keyboard's isr to handle all possible interrupts because fuck it
	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler; 

	printk("%lx\n", keyboard_handler);
	printk("%lx\n", keyboard_address);

	int i;
	for(i = 0; i < IDT_SIZE; i++){
	    IDT_Entry *entry = IDT + i;
	    entry->funct_ptr_1 = (uint64_t) ISRs[i] & 0x000000000000FFFF;
	    entry->gdt_select = 0x08;
	    entry->ist = 0;
	    entry->res = 0;
	    entry->type = 0xF;
	    entry->must_be_0 = 0;
	    entry->priv_lvl = 0;
	    entry->present = 1;
	    entry->funct_ptr_2 = ((uint64_t) ISRs[i] & 0x00000000FFFF0000) >> 16;
	    entry->funct_ptr_3 = ((uint64_t) ISRs[i] & 0xFFFFFFFF00000000) >> 32;
	    entry->reserved = 0;
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

	//now done with initialization

	//mask interrupts
	outby(0x21, 0xff);
	outby(0xA1, 0xff);

	/* fill the IDT descriptor */
	//idt_address = (unsigned long)IDT;
	//idt_ptr[0] = (sizeof (IDT_Entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	//idt_ptr[1] = idt_address >> 16 ;

    struct {
        uint16_t length;
        void*    base;
	} __attribute__((packed)) IDTR = { sizeof(IDT_Entry) * 256, IDT };

	asm ( "lidt %0" : : "m"(IDTR) );

	//load_idt((unsigned long)IDTR);
}

void kb_init(void){
	//mask all interrupts to off except for the keyboard IRQ1
	outby(0x21, 0xfd);
	outby(0xA1, 0x00);
	asm ( "sti" );
}

void generic_c_isr(int irq, int err){
	// printk("received keypress\n");
	(c_ISRs[irq])(irq, err);

	if(irq > 8){
		outby(0xA0, 0x20);
	}
	outby(0x20, 0x20);
}