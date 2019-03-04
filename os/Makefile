# ===== BUILD CONFIGURATION VARIABLES =====
# default target architecture
arch ?= x86_64
vm_prog ?= qemu-system-x86_64

loop_dev_1 ?= /dev/loop6
loop_dev_2 ?= /dev/loop7

fat_bs ?= 512
fat_num_blocks ?= 196575
fat_blank_img ?= .cache/fat_templates/blank_fat_$(fat_bs)_$(fat_num_blocks).img

# ===== COMPILER CONFIGURATION =====
incl_path := $(shell pwd)/src/arch/x86_64/

# cross-compiler
CC=~/opt/cross64/x86_64-elf-4.9.1-Linux-x86_64/bin/x86_64-elf-gcc

# ===== BUILD UTILITY FILES =====
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
.PHONY: all clean run iso fat

all: $(kernel)

# tear down everything set up during build of kernel
clean:
	-killall gdb # kill all gdb processes
	-killall $(vm_prog) # kill virtual machine running the OS
	-sudo umount /mnt/fatgrub
	-sudo losetup -d $(loop_dev_1)
	-sudo losetup -d $(loop_dev_2)
	-rm -rf build
	-rm $(symbol_link_file)


run: $(iso)
	sudo $(vm_prog) -machine ubuntu -s -cdrom $(iso) -serial stdio

runf: $(fat)
	@sudo $(vm_prog) -machine ubuntu -s -drive format=raw,file=$(fat) -serial stdio

runb: $(iso)
	@bochs -q

iso: $(iso)

$(fat_blank_img):
	mkdir -p .cache/fat_templates
	dd if=/dev/zero of=$(fat_blank_img) bs=$(fat_bs) count=$(fat_num_blocks)

fat: $(fat)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue --recheck -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

$(fat): $(kernel) $(grub_cfg) $(fat_blank_img) $(is_superuser)
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	cp $(fat_blank_img) $(fat)
	parted $(fat) mklabel msdos
	parted $(fat) mkpart primary fat32 2048s 30720s
	parted $(fat) set 1 boot on
	sudo losetup $(loop_dev_1) $(fat)
	sudo losetup $(loop_dev_2) $(fat) -o 1048576
	sudo mkdosfs -F32 -f 2 $(loop_dev_2)
	sudo mount $(loop_dev_2) /mnt/fatgrub
	sudo grub-install --root-directory=/mnt/fatgrub --no-floppy --recheck --modules="normal part_msdos ext2 multiboot2" $(loop_dev_1)
	sudo cp -r build/isofiles/* /mnt/fatgrub
	sudo cp "test.txt" /mnt/fatgrub/test.txt
	sudo cp "test.txt" /mnt/fatgrub/test2.txt
	sudo cp "test.txt" /mnt/fatgrub/test3.txt
	sudo cp "test.txt" /mnt/fatgrub/test4.txt
	sudo cp "test.txt" /mnt/fatgrub/test5.txt
	sudo cp "test.txt" /mnt/fatgrub/test6.txt
	sudo cp "test.txt" /mnt/fatgrub/test7.txt
	sudo cp "test.txt" /mnt/fatgrub/test8.txt
	sudo cp "test.txt" /mnt/fatgrub/test9.txt
	sudo cp "test.txt" /mnt/fatgrub/test10.txt
	sudo cp "test.txt" /mnt/fatgrub/test11.txt
	sudo cp "test.txt" /mnt/fatgrub/test12.txt
	sudo cp "test.txt" /mnt/fatgrub/test13.txt
	sudo cp "test.txt" /mnt/fatgrub/test14.txt
	sudo cp "test.txt" /mnt/fatgrub/test15.txt
	sudo cp "test.txt" /mnt/fatgrub/test16.txt
	sudo cp "test.txt" /mnt/fatgrub/test17.txt
	sudo cp "test.txt" /mnt/fatgrub/test18.txt
	sudo cp "test.txt" /mnt/fatgrub/test19.txt
	sudo cp "test.txt" /mnt/fatgrub/test20.txt
	sudo cp "test.txt" /mnt/fatgrub/test21.txt
	sudo cp "test.txt" /mnt/fatgrub/test22.txt
	sudo cp "test.txt" /mnt/fatgrub/test23.txt
	sudo cp "test.txt" /mnt/fatgrub/test24.txt
	sudo cp "test.txt" /mnt/fatgrub/test25.txt
	sudo cp "test.txt" /mnt/fatgrub/test26.txt
	sudo cp "test.txt" /mnt/fatgrub/test27.txt
	sudo cp "test.txt" /mnt/fatgrub/test28.txt
	sudo cp "test.txt" /mnt/fatgrub/test29.txt
	sudo cp "test.txt" /mnt/fatgrub/test30.txt
	sudo cp "test.txt" /mnt/fatgrub/test31.txt
	sudo cp "test.txt" /mnt/fatgrub/test32.txt
	sudo echo "blaaaaaaaah" > /mnt/fatgrub/zzzzzzzzzzzzzz.txt
	sudo echo "blaaaaaaaah" > /mnt/fatgrub/testing123.abc
	sudo echo "blaaaaaaaah" > /mnt/fatgrub/AAAAAA.txt
	sudo mkdir /mnt/fatgrub/test_dir
	sudo echo "test" > /mnt/fatgrub/a.t
	ls -l --full-time /mnt/fatgrub > fatgrub.ls
	sudo umount /mnt/fatgrub
	losetup -d $(loop_dev_1)
	losetup -d $(loop_dev_2)
	rm -rf build/isofiles

$(kernel): $(assembly_object_files) $(c_object_files) $(linker_script)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files)
	@-ln -s $(kernel) $(symbol_link_file)

# compile assembly files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

#compile c files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.c
	@mkdir -p $(shell dirname $@)
	@$(CC) -c -g -I$(incl_path) $< -o $@ -Wall -Werror -fno-builtin -mno-red-zone
