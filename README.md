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
#### Milestones that still need to be implemented:
9. Virtual Page Allocator
    1. Virtual Memory Layout (initially attempting to copy the structure from the notes)
        1. PML4E SLOT 0: Physical Page Frames
        2. PML4E SLOT 1: Kernel Stacks
        3. PML4E SLOT 2-14: Reserved/Growth
        4. PML4E SLOT 15: Kernel Heap
        5. PML4E SLOT 16: Beginning of User space
    2. Need the following helper functions to access and manipulate the page table
        1. void init_page_table(void \*page_table);
            1. This function takes a pointer to an uninitialized page table structure and correctly sets up all levels of the structure
            2. Considering that the table is not necessarily contiguous memory, I might have to rethink this considering the fact that I don't have memory allocation implemented
        2. int walk_and_verify_page_table(void \*page_table);
            1. This function will primarily be used to debug the page table (not sure how else this could be used, but it is indicated that this is needed)
            2. Will return 0 if there is no error and 1 if something is wrong
        3. 
10. Heap allocator
11. Cooperative Multitasking
12. Process Management and Keyboard Driver

#### Things that need to be done/fixed in current code:
* Move everything not pertaining to booting into long mode into separate files
* Figure out why exclamation marks are randomly printed to serial output
####Miscellaneous notes####
* Figure out what inline function tag does
=======
* Make sure that there are no race conditions in the implemented code
