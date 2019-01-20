# Build Information
This file contains the following information:

* Pertinent terminology
* Overview of the procedure followed to build a bootable kernel image.
* An overview of the tools used to build this project.
* The commands used to build, run, and debug the OS.

------

## OS Build Terminology
| Term | Context | Explanation | Sources |
| :--- | ------- | ----------- | ------- |
| Kernel | General Software | todo | todo |
| Boot-Loader | General Software | First software program that runs when a computer starts. Loads and transfers control to an operating system kernel (such as `Linux` or `GNU Mach`). The kernel, in turn, then initializes the rest of the operating system. | `info grub` |
| Loop Device | Systems | "Pseudo device (actually just a file) that acts as a block-based device. You want to mount a file (disk1.iso) that will act as entire filesystem, so you use loop." | [StackOverflow](https://github.com/jadams41/Senior-Project/edit/master/os/docs/building/README.md)|
| `grub` | Project's Primary Boot-Loader utility | `GRand Unified Bootloader`. Capable of loading a wide variety of operating systems. Flexible: you can load an arbitrary operating system the way you like, without recording the physical position of your kernel on the disk (just specify kernel file name and the drive and partition where it resides). | `info grub` |

## Build Procedure
General build procedure steps:

#### 1. Cross-compile source files into object files
_Compiling `.c` source:_

```bash
> x86_64-elf-gcc \ #cross-compiler binary
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
> nasm \ #"Netwide Assembler, a portable 80x86 assembler."
	-felf64 #output file will be formatted as "ELF64 (x86_64) object files"
```

#### 2. Link object files into kernel binary

_Link object files into kernel binary_
```bash
> ld \ # The GNU Linker
	-n \ #Turn off page alignment of sections, and disable linking against shared libraries.
 	-T <linker_script> \ #Replace ld's default linkerscript with <linker_script>.
	-o <kernel_binary_outfile> \ #output linked file to <kernel_binary_outfile>
	<compiled_asm_obj_files> \ #all compiled assembly files to link into the binary
	<compiled_c_obj_files> #all compiled c files to link into the binary
```

#### 3. Create a bootable image (containing the kernel binary)
There are multiple supported bootable images which can be created, each is explained separately below.

###### `.iso`
"An `ISO` image file is a snapshot of the data and layout of a CD or DVD, saved in `ISO-9660` format." ([source](http://www.ntfs.com/bootdisk_quest_isofiles.htm))

Notes:
* Building this bootable image is relatively fast.
* This image is passed to the virtual machine with the `-cdrom` option. The virtual machine handles this image as if it were a physical CD loaded into the computer's optical drive.
* "ISO 9660 is by design a read-only file system. This means that all the data has to be written in one go to the medium. Once written, there is no provision for altering the stored content. Therefore ISO 9660 is not suitable to be used on random-writable media, such as hard disks." ([source](https://unix.stackexchange.com/questions/26237/iso-file-readonly)) **Meaning that there is no way to support a R/W file-system when booting the kernel from an `.iso` image**

_Creating a bootable `.iso`_

```bash
# grub-mkrescue converts a file-tree into a bootable image, so first need to setup and populate the directory structure
# grub rescue iso file-tree:
# build/isofiles
# └── boot
#      ├── grub
#      │   └── grub.cfg
#      └── kernel.bin
> mkdir -p build/isofiles/grub
> cp $kernel build/isolfiles/boot/kernel.bin
> cp $grub_cfg build/isofiles/grub

# convert the populate file-tree into a bootable iso image
> grub-mkrescue -o $iso_outfile build/isofiles

# clean up file-tree after generating the image
> rm -r build/isofiles
```

###### `.img`
"Designed to create a backup copy of a floppy disk in a single file. It works by creating a bitmap of each sector of the disk that has been written to. As these sectors are 512 bytes in size, IMG files are always sized in multiples of 512 bytes. Since the demise of floppy disks, the `IMG` format has been used for the creation of hard disk image files." ([source](https://www.techwalla.com/articles/what-are-the-differences-between-iso-and-img-files) 

Notes:
* Generating this bootable image is significantly more complex and time consuming than generating an `.iso`.
* `sudo` permissions are required to create an `.img`.
* This image format allows for `r/w` access, which makes it much more suitable for supporting useful filesystems.

_Creating a bootable `.img`_ (with `FAT32` filesystem)
```bash
# CREATE DIRECTORY STRUCTURE TO BE ARCHIVED ONTO THE `.IMG` FILE
> mkdir -p build/isofiles/boot/grub
> cp $kernel build/isofiles/boot/kernel.bin
> cp $(grub_cfg) build/isofiles/boot/grub

# CREATE BLANK `.IMG`
# create 0'd-out file which will be formatted into the image (total size = 16MiB)
> dd if=/dev/zero of=$(fat) bs=512 count=32768

# CREATE PARTITION ON BLANK `.IMG`
# create a new disklabel of label-type=`msdos`
> parted $(fat) mklabel msdos

# add a `primary` partition of type `fat32` starting at 2048s and ending at 30720s to the partition table on the `.img`.
> parted $(fat) mkpart primary fat32 2048s 30720s

# set flag bootable to 1 for the partition
> parted $(fat) set 1 boot on

# MOUNT AND WRITE TO `.IMG`
# setup created `.img` file as a loop device #2
> sudo losetup /dev/loop2 $(fat)

# setup created `.img` file (starting after first 1048576 bytes (beginning of the fs)) as loop device #3
> sudo losetup /dev/loop3 $(fat) -o 1048576

# create `FAT32` filesystem on `.IMG` (starting after the first 1048576 bytes)
> sudo mkdosfs -F32 -f 2 /dev/loop3

# mount newly created filesystem
> sudo mount /dev/loop3 /mnt/fatgrub

# install grub bootloader on beginning of `.IMG` 
> sudo grub-install \
	--root-directory=/mnt/fatgrub \ #`.IMG`s filesystem mounted. todo can't find information about this option
	--no-floppy \ #todo can't find information about this option
	--modules="normal part_msdos ext2 multiboot" \ #pre-load specified modules
	/dev/loop2 #this loop device is pointing to the beginning of the `.IMG`

# copy file-tree (containing kernel binary and grub config file) into the `.IMG`'s FAT32 filesystem
> sudo cp -r build/isofiles/* /mnt/fatgrub

# TEARDOWN
# unmount `.IMG`'s filesystem from host machine
> sudo umount /mnt/fatgrub

# detach loop device pointing to beginning of `.IMG`
> losetup -d /dev/loop2
```

#### 4. Booting the Generated Image
Image is booted onto a virtual machine (using `qemu-system-x86_64`). The commands to do this differ based on the type of bootable image which was generated. Here are the commands for each:

###### `.iso`
```bash
> qemu-system-x86_64 \
	-s \ #open a gdbserver on `tcp::1234`
	-cdrom <generated.iso> \ #load `.iso` as cdrom on the vm
	-serial stdio #connect vm's serial port to stdio
```

###### `.img` (with `FAT32` filesystem)
```bash
> qemu-system-x86_64 \
	-s \ #open a gdbserver on `tcp::1234`
	-drive format=raw,file=<generated.img> \ #load `.img` as a drive on the vm
	-serial stdio #connect vm's serial port to stdio
```


#### 5. Running the kernel
_Once the image has been booted, the kernel will hang until `gdb` connection is established and `enabled` is set to 1._

Connecting with `gdb` and enabling kernel execution:
```bash
> gdb

# once inside of gdb
gdb$ set arch i386:x86-64:intel
gdb$ target remote localhost:1234
gdb$ symbol-file symbol.bin
gdb$ set variable enabled=1
```

------


## Tools
| name | function | project's use |
| ---- | -------- | ------------- |
| `gcc` | compiler | used to compile all project `.c` source files |
| `nasm` | assembler | used to assemble all project `.asm` source files |
| `make` | build management | used to build codebase and run produced binaries |
| `mount` | external filesystem interaction | create and modify bootable images |
| `losetup` | bootable image interaction | setup and control of loop devices. Used to mount "filesystem" files as loop devices (which allows for block-device interaction) |
| `mkdosfs` | filesystem creator | format `.img` file to be read as a `FAT32` filesystem |
| `grub-install` | grub utility |  install grub on `.img` file |
| `grub-mkrescue` | grub utility | generate grub rescue image (`.iso`) which contains supplied files and grub |
|`dd` | file utility | used to generate blank `.img` file with specific size |
| `parted` | partition manipulation program | Used to format partition information on `.img` file. |

------
