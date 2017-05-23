This is the repository containing the implementation of a x86_64 operating system for CPE454.

### Status
#### Currently I have the following milestones complete:
1. ~~Setup Development environment~~
2. ~~Boot OS in Long Mode~~
3. ~~VGA text mode and printk~~
4. ~~Polling keyboard driver (now removed with interrupt implementation)~~
5. ~~Handle Interrupts~~
6. ~~Interrupt-Driven Serial Output~~
7. ~~Interrupt Stack Table Working~~
8. ~~Parse Multiboot tags and Page Frame Allocator~~
9. ~~Virtual Page Allocator~~
    * Errors in the virtual page Allocator
        * Currently general protection faulting in the page_fault_handler when trying to page on demand
            * Looks like this is due to the fact that the page indexes are being grabbed correctly but they are not being shifted back
            * I.E 0b11110000 grabbing 0b1100000 but not shifting back to 0b11
        * Fixed the previous error however, on the first time that there is a page fault, p4[15](heap addresses) is set correctly, however that p3 table is not set correctly (not sure if I planned for this)
        * This is ultimately causing the table_pointer to be trashed
        * The main error with #pf triggering #gp was that I was not grabbing error code off of the stack, now that I am doing this, everything with page fault and subsequently on demand paging is working
#### Milestones that still need to be implemented:
10. Heap allocator
11. Cooperative Multitasking
12. Process Management and Keyboard Driver

#### Things that need to be done/fixed in current code:
* Move everything not pertaining to booting into long mode into separate files
* ~~Figure out why exclamation marks are randomly printed to serial output~~
    * There was a problem in the consumer checking if at the end of the buffer
    * Byte immediately following held data pertaining to an exclamation mark
* Make sure that the error codes for exceptions are grabbed an passed into the irq
    * I think that I am not following his specs to have a void * argument which can hold extra info (could be very useful to hold cr2 and cr3 for page faults)
    * Implemented this for #pf but still need to do for all other excpetions that push error codes
####Miscellaneous notes####
* Figure out what inline function tag does
=======
* Make sure that there are no race conditions in the implemented code
