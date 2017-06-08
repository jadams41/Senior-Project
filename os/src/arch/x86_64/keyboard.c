#include "keyboard.h"
#include "memoryManager.h"
#include "printk.h"

static KBD_state kbd;

void init_kbd_state(){
    kbd.read = kbd.kbd_buffer;
    kbd.write = kbd.kbd_buffer;
    kbd.buf_end = kbd.kbd_buffer + 160;
    PROC_init_queue(&kbd.blocked);
}

char KBD_read(){
    asm("CLI");
    while(kbd.read == kbd.write){
        PROC_block_on(&kbd.blocked, 1);
        asm("CLI");
    }
    char toRet = *kbd.read++;
    if(kbd.read >= kbd.buf_end){
        kbd.read = kbd.kbd_buffer;
    }
    kbd.buffLen -= 1;

    asm("STI");

    return toRet;
}

// function is only meant to be called from the keyboard isr
void KBD_write(char written){
    if(kbd.buffLen == 160){
        printk_err("keyboard buffer is out of space\n");
        return;
    }
    *kbd.write++ = written;
    if(kbd.write >= kbd.buf_end){
        kbd.write = kbd.kbd_buffer;
    }
    kbd.buffLen += 1;
    if(&kbd.blocked && kbd.blocked.head)
        PROC_unblock_head(&kbd.blocked);
}
