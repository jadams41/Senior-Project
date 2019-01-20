# ===== BUILD CONFIGURATION VARIABLES =====
# default target architecture
arch ?= x86_64
vm_prog ?= qemu-system-x86_64

loop_dev_1 ?= /dev/loop2
loop_dev_2 ?= /dev/loop3

fat_bs ?= 512
fat_num_blocks ?= 98304 #was 32768, but was complaining "not enough clusters for a 32-bit fat"



# ===== BUILD UTILITY FILES =====
# cross-compiler
CC=~/opt/cross64/x86_64-elf-4.9.1-Linux-x86_64/bin/x86_64-elf-gcc

# linker script used to arrange the compiled kernel object files into a proper kernel binary 
linker_script := src/arch/$(arch)/linker.ld

# file used to configure grub when generating OS image
grub_cfg := src/arch/$(arch)/grub.cfg


# ===== OUTPUT FILES =====
# name of the generated kernel binary
kernel := build/kernel-$(arch).bin
# name of symlink to most recently built kernel binary
symbol_link_file := symbol.bin

# OS image generated when building CD-ROM (read only file system)
iso := build/os-$(arch).iso
# OS image generated when building with a FAT file system (supports read/write)
fat := build/os-$(arch).img


# ===== SOURCE AGGREGATE TARGETS =====
# all assembly source files
assembly_source_files := $(shell find src/arch/$(arch)/ -name "*.asm")
# compiled assembly object files
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
	build/arch/$(arch)/%.o, $(assembly_source_files))

# all c source files
c_source_files := $(shell find src/arch/$(arch)/ -name "*.c")
# compiled c object files
c_object_files := $(patsubst src/arch/$(arch)/%.c, \
	build/arch/$(arch)/%.o, $(c_source_files))

# targets which do not produce file with name of target
.PHONY: all clean run iso

all: $(kernel)

# tear down everything set up during build of kernel
clean:
	-killall gdb # kill all gdb processes
	-killall $(vm_prog) # kill virtual machine running the OS
	-sudo umount /mnt/fatgrub
	-sudo losetup -d $(loop_dev_1)
	-sudo losetup -d $(loop_dev_2)
	@rm -rf build
	-rm $(symbol_link_file)


run: $(iso)
	@$(vm_prog) -s -cdrom $(iso) -serial stdio

runf: $(fat)
	@$(vm_prog) -s -drive format=raw,file=$(fat) -serial stdio

runb: $(iso)
	@bochs -q

iso: $(iso)

fat: $(fat)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

$(fat): $(kernel) $(grub_cfg)
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	dd if=/dev/zero of=$(fat) bs=$(fat_bs) count=$(fat_num_blocks)
	parted $(fat) mklabel msdos
	parted $(fat) mkpart primary fat32 2048s 30720s
	parted $(fat) set 1 boot on
	sudo losetup $(loop_dev_1) $(fat)
	sudo losetup $(loop_dev_2) $(fat) -o 1048576
	sudo mkdosfs -F32 -f 2 $(loop_dev_2)
	sudo mount $(loop_dev_2) /mnt/fatgrub
	sudo grub-install --root-directory=/mnt/fatgrub --no-floppy --modules="normal part_msdos ext2 multiboot" $(loop_dev_1)
	sudo cp -r build/isofiles/* /mnt/fatgrub
	sudo umount /mnt/fatgrub
	losetup -d $(loop_dev_1) #todo: figure out if I should be doing this for loop_dev_2 as well

$(kernel): $(assembly_object_files) $(c_object_files) $(linker_script)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files)
	@ln -s $(kernel) $(symbol_link_file)

# compile assembly files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

#compile c files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.c
	@mkdir -p $(shell dirname $@)
	@$(CC) -c -g $< -o $@ -Wall -Werror -fno-builtin -mno-red-zone
