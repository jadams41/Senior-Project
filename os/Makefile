# ===== BUILD CONFIGURATION VARIABLES =====
# default target architecture
arch ?= x86_64
vm_prog ?= qemu-system-x86_64

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
	-sudo losetup -d /dev/loop2
	-sudo losetup -d /dev/loop3
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
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@dd if=/dev/zero of=$(fat) bs=512 count=32768
	@parted $(fat) mklabel msdos
	@parted $(fat) mkpart primary fat32 2048s 30720s
	@parted $(fat) set 1 boot on
	@echo "about to run losetup /dev/loop2 $(fat)"
	@sudo losetup /dev/loop2 $(fat)
	@echo "about to run losetup /dev/loop3 $(fat)"
	@sudo losetup /dev/loop3 $(fat) -o 1048576
	@sudo mkdosfs -F32 -f 2 /dev/loop3
	@sudo mount /dev/loop3 /mnt/fatgrub
	@sudo grub-install --root-directory=/mnt/fatgrub --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop2
	@sudo cp -r build/isofiles/* /mnt/fatgrub
	@sudo umount /mnt/fatgrub
	@losetup -d /dev/loop2


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
