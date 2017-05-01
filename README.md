This is the repository containing the implementation of a x86_64 operating system for CPE454.

Currently I have the following milestones complete:
1. Setup Development environment
2. Boot OS in Long Mode
3. VGA text mode and printk
4. Polling keyboard driver (now removed with interrupt implementation)
5. Handle Interrupts (Interface is only partially implemented)
6. Interrupt-Driven Serial Output (Interface is only partially implemented)

Things that need to be done/fixed in current code:
* Move everything not pertaining to booting into long mode into separate files
* Fully implement the Interrupt Interface (requires the implementation of the following functions)
  * void IRQ_set_mask(int irq);
  * void IRQ_clear_mask(int irq);
  * int IRQ_get_mask(int IRQLine); //no idea what the fuck this is
  * void IRQ_end_of_interrupt(int irq); //again, no idea what the fuck this is
  * void IRQ_set_handler(int irq, irq_handler_t handler, void *arg); //will need to modify my irqs to do something with args I think
  * **Need to implement the TSS's this will constitute the following**
  	* figure out what Interrupts (faults) need TSS
  	* Figure out how the TSS will be initialized (think that this is just another static chunk in one of the .ASM files)
  	* Probably also figure out where the TSS stacks will be and how they will be implemented
  		* Again, I think that this is just some more asm define bytes and tags

* Make sure that there are no race conditions in the implemented code

Milestones that still need to be implemented:
7. Parse Multiboot tags and Page Frame Allocator
8. Virtual Page Allocator
9. Heap allocator
10. Cooperative Multitasking
11. Process Management and Keyboard Driver

####Miscellaneous notes####
* Figure out what inline function tag does