set arch i386:x86-64:intel
target remote localhost:1234
symbol-file symbol.bin
set variable enabled=1
skip printk
skip inb
skip outb
skip entry_present
skip strip_present_bits
