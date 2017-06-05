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
10. ~~Heap allocator (kmalloc)~~
    * Largely (emphasis on largely) untested
    * however, can't think of too many edge cases
11. ~~Cooperative Multitasking~~
    * There is one main shitty thing in the virtual page allocator that was patched up during the building of this milestone, however needs to be fixed
        * Previously the only place we could allocate memory was in the kernel heap, however when making stacks for user processes, I needed to hack around that big chunk of code in order to get the functionality that I needed
        * In doing this, I simply changed the local variable holding the pml4 index before and after allocating, however this means that if I allocate 5 kheap pages and then 1 userspace page, it will start at the 6th page in userspace. 
#### Milestones that still need to be implemented:
12. Process Management and Keyboard Driver

#### Things that need to be done/fixed in current code:
* Move everything not pertaining to booting into long mode into separate files
* ~~Figure out why exclamation marks are randomly printed to serial output~~
    * There was a problem in the consumer checking if at the end of the buffer
    * Byte immediately following held data pertaining to an exclamation mark
* Make sure that the error codes for exceptions are grabbed an passed into the irq
    * I think that I am not following his specs to have a void * argument which can hold extra info (could be very useful to hold cr2 and cr3 for page faults)
    * Implemented this for #pf but still need to do for all other excpetions that push error codes
#### Miscellaneous notes:
* Figure out what inline function tag does
* Make sure that there are no race conditions in the implemented code

#### Questions for the professor:
* Ask if the way that I'm doing virtual address allocation sucks (right now I loop through all of the pages starting at the beginning until I find an empty one)
    * It seems like the alternative would be to just keep a reference to the last allocated entry in the p1 table and send back the next one regardless of if things were freed or not
    * <b>This is in the same vein</b> Should I be checking if the entire p1 table is empty when calling MMU_free_page (this would happen a lot more in the other implementation) and if so, freeing that portion of the page table
* Figure what to do if kmalloc(\<very big\>) is called
    * Right now anything > largest block size (2048) is just ignored
    * However, after reading his documentation, he might be suggesting to just allocate raw virtual pages to satisfy the request

#### Known issues
* When the program runs out physical memory and a page fault is triggered on a page that is attempted to be brought in on demand, the program will halt in the page fault handler
    * This is exploited in the kmalloc test when we attempt to exhaust the memory, considering kmalloc grabs a virtual address
    * Looked into what happens in linux and when kmalloc runs out of memory, just get a message that says killed and the program quits
