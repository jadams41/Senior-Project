# Build Information
This file contains the following information:

* Pertinent terminology
* Overview of the procedure followed to build a bootable kernel image.
* An overview of the tools used to build this project.
* The commands used to build, run, and debug the OS.

------

## Build Procedure
General build procedure steps:

#### 1. Cross-compile source files into object files
_Compiling `.c` source:_

```bash
$ x86_64-elf-gcc \ #cross-compiler binary
	-c \ #compile without linking
	-g \ #produce debugging information (so executable can be understood by gdb)
	<src_x.c> \ #same command run for each c source file
	-Wall \ #all warnings enabled
	-Werror \ #all warnings treated as errors
	-fno-builtin \ #Don't use information about built-in functions (such as printf, strlen, malloc, etc.) to generate compiler warnings (important because these functions have nonstandard implementation)
	-mno-red-zone #Disable "red-zone" for x86-64 code. Red zone is reserved 128-byte area beyond the location of the stack pointer that is not modified by singal or interrupt handlers and therefore can be used for temporary data without adjusting the stack pointer.
```

_Compiling `.asm` source:_

```bash
$ nasm \ #"Netwide Assembler, a portable 80x86 assembler."
	-felf64 #output file will be formatted as "ELF64 (x86_64) object files"
```

#### 2. 

------

## Tools
* `gcc` - compiler
* `make` - build system
* `mount` - creating images and mounting images
* `losetup` - setup and control loop devices
* `mkdosfs` - create an MS-DOS filesystem under Linux
* `grub-install` - 
* `grub-mkrescue` - 
* `dd` - convert and copy a file

------
