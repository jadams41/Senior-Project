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
