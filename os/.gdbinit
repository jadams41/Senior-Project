set arch i386:x86-64:intel
target remote localhost:1234
symbol-file build/symbol.bin
set variable enabled=1

# skip all print functions
skip printk
skip printk_err
skip printk_warn
skip printk_info
skip printk_dir

# skip memory allocation
skip kmalloc
skip dma_alloc_coherent

# skip mmio
skip inb
skip inw
skip inl
skip outb
skip outw
skip outl

skip entry_present
skip strip_present_bits
