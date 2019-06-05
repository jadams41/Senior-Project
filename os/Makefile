#!/usr/bin/colormake

# ===== BUILD CONFIGURATION VARIABLES =====
# default target architecture
arch ?= x86_64
vm_prog ?= qemu-system-x86_64

fat_bs ?= 512
fat_num_blocks ?= 196575
fat_blank_img ?= .cache/fat_templates/blank_fat_$(fat_bs)_$(fat_num_blocks).img

# ===== RUNTIME CONFIGURATION VARIABLES =====
# default static ip address
STATIC_IP ?= "\"172.16.210.187\""

# random seed
RAND_SEED = 15

# ===== COMPILER CONFIGURATION =====
incl_path := $(shell pwd)/src/arch/x86_64/

# cross-compiler
CC_OPT="-fdiagnostics-color=always"
CC=~/opt/cross64/x86_64-elf-4.9.1-Linux-x86_64/bin/x86_64-elf-gcc

# ===== PROJECT STRUCTURE =====
build_dir := build
build_debug_dir := $(build_dir)/debug_outputs

# ===== BUILD UTILITY FILES =====
# linker script used to arrange the compiled kernel object files into a proper kernel binary
linker_script := src/arch/$(arch)/linker.ld

# file used to configure grub when generating OS image
grub_cfg := src/arch/$(arch)/grub.cfg


# ===== OUTPUT FILES =====
# name of the generated kernel binary
kernel := $(build_dir)/kernel-$(arch).bin
# name of symlink to most recently built kernel binary
symbol_link_file := $(build_dir)/symbol.bin

# OS image generated when building CD-ROM (read only file system)
iso := $(build_dir)/os-$(arch).iso
# OS image generated when building with a FAT file system (supports read/write)
fat := $(build_dir)/os-$(arch).img

# symbolic links to loopback devices
ld1_sym := $(build_dir)/loop_dev_1
ld2_sym := $(build_dir)/loop_dev_2

# ===== SOURCE AGGREGATE TARGETS =====
# all assembly source files
assembly_source_files := $(shell find src/arch/$(arch)/ -name "*.asm")
# compiled assembly object files
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
	$(build_dir)/arch/$(arch)/%.o, $(assembly_source_files))

# all c source files
c_source_files := $(shell find src/arch/$(arch)/ -name "*.c")
# compiled c object files
c_object_files := $(patsubst src/arch/$(arch)/%.c, \
	$(build_dir)/arch/$(arch)/%.o, $(c_source_files))

# targets which do not produce file with name of target
.PHONY: all clean run iso fat generate_random_seed

all: $(kernel)

# tear down everything set up during build of kernel
clean:
	-killall gdb # kill all gdb processes
	-killall $(vm_prog) # kill virtual machine running the OS
	-sudo umount /mnt/fatgrub
	-rm -rf build

$(build_dir):
	mkdir -p $(build_dir)

$(build_debug_dir): $(build_dir)
	mkdir -p $(build_debug_dir)

run: $(iso)
	sudo $(vm_prog) \
	-machine ubuntu \
	-s -cdrom $(iso) \
	-serial stdio \
	-net nic,model=rtl8319 \
	-net socket,listen=:8010 \
	-no-kvm-irqchip

generate_random_seed: FORCE
	$(eval RAND_SEED := $(shell bash -c 'echo \"$$RANDOM\"'))
	@echo "Random seed = $(RAND_SEED)"

FORCE:

#-net bridge,br=br000
# -netdev tap,id=mynet0,ifname=qtap0,script=no,downscript=no
# ,netdev=mynet0,macaddr=52:55:00:d1:55:01
# -net bridge,br=virbr0 
runf: $(fat)
	#sudo /etc/qemu-ifup tap5
	@sudo $(vm_prog) \
	-machine ubuntu \
	-s -drive format=raw,file=$(fat) \
	-serial stdio \
	-netdev tap,id=mynet0,ifname=tap6,script=no,downscript=no \
	-net nic,model=rtl8139,netdev=mynet0 \
	-no-kvm-irqchip
	#sudo /etc/qemu-ifdown tap5

runb: $(iso)
	@bochs -q

runlinux:
	@sudo $(vm_prog) \
	-boot d \
	-cdrom /dev/cdrom \
	-netdev tap,id=mynet0,ifname=tap6,script=no,downscript=no \
	-net nic,model=rtl8139,netdev=mynet0 \
	-m 512 -hda mydisk.img

iso: $(iso)

$(fat_blank_img):
	mkdir -p .cache/fat_templates
	dd if=/dev/zero of=$(fat_blank_img) bs=$(fat_bs) count=$(fat_num_blocks)

fat: $(fat)

$(iso): $(build_dir) $(build_debug_dir) $(kernel) $(grub_cfg)
	@mkdir -p $(build_dir)/isofiles/boot/grub
	@cp $(kernel) $(build_dir)/isofiles/boot/kernel.bin
	@cp $(grub_cfg) $(build_dir)/isofiles/boot/grub
	@grub-mkrescue --recheck -o $(iso) $(build_dir)/isofiles 2> /dev/null
	@rm -r $(build_dir)/isofiles

$(fat): $(build_dir) $(build_debug_dir) $(kernel) $(grub_cfg) $(fat_blank_img) $(is_superuser)
	mkdir -p $(build_dir)/isofiles/boot/grub
	cp $(kernel) $(build_dir)/isofiles/boot/kernel.bin
	cp $(grub_cfg) $(build_dir)/isofiles/boot/grub
	cp $(fat_blank_img) $(fat)
	ln -s `sudo losetup --show -f $(fat)` $(ld1_sym)
	ln -s `sudo losetup --show -f $(fat) -o 1048576` $(ld2_sym)

	parted $(fat) mklabel msdos
	parted $(fat) mkpart primary fat32 2048s 30720s
	parted $(fat) set 1 boot on
	sudo mkdosfs -F32 -f 2 $(shell readlink -f $(ld2_sym))
	sudo mount $(shell readlink -f $(ld2_sym)) /mnt/fatgrub
	sudo grub-install --root-directory=/mnt/fatgrub --no-floppy --recheck --modules="normal part_msdos ext2 multiboot2" $(shell readlink -f $(ld1_sym))
	sudo cp -r $(build_dir)/isofiles/* /mnt/fatgrub
	sudo echo "Contents of the really long file" > /mnt/fatgrub/this_is_a_really_long_filename-HERE_ARE_SOME_CAPITAL_LETTERS.txt
	sudo echo "Hidden File contents" > /mnt/fatgrub/.my_hidden_file.txt
	tree -a /mnt/fatgrub > $(build_debug_dir)/fatgrub.tree # store a copy of the OS's initial file system
	sudo umount /mnt/fatgrub
	losetup -d $(shell readlink -f $(ld1_sym))
	rm $(ld1_sym)
	losetup -d $(shell readlink -f $(ld2_sym))
	rm $(ld2_sym)
	rm -rf $(build_dir)/isofiles

$(kernel): $(assembly_object_files) $(c_object_files) $(linker_script)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files)
	ln -s --force ../$(kernel) $(symbol_link_file)

# compile assembly files
$(build_dir)/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

#compile c files
$(build_dir)/arch/$(arch)/%.o: src/arch/$(arch)/%.c generate_random_seed
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CC_OPT) -c -g -I$(incl_path) -DSTATIC_IP=$(STATIC_IP) -DRAND_SEED=$(RAND_SEED) $< -o $@ -Wall -Werror -fno-builtin -mno-red-zone
