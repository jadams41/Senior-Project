arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
fat := build/os-$(arch).img

CC=~/opt/cross64/x86_64-elf-4.9.1-Linux-x86_64/bin/x86_64-elf-gcc

linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg


assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
	build/arch/$(arch)/%.o, $(assembly_source_files))

c_source_files := $(shell find src/arch/$(arch)/ -name "*.c")
c_object_files := $(patsubst src/arch/$(arch)/%.c, \
	build/arch/$(arch)/%.o, $(c_source_files))

symbol_link_file := symbol.bin

.PHONY: all clean run iso

all: $(kernel)

clean:
	-killall gdb
	-killall qemu-system-x86_64
	-sudo umount /mnt/fatgrub
	-sudo losetup -d /dev/loop0
	-sudo losetup -d /dev/loop1
	@rm -rf build
	-rm $(symbol_link_file)


run: $(iso)
	@qemu-system-x86_64 -s -cdrom $(iso) -serial stdio

runf: $(fat)
	@qemu-system-x86_64 -s -drive format=raw,file=$(fat) -serial stdio

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
	@sudo losetup /dev/loop0 $(fat)
	@sudo losetup /dev/loop1 $(fat) -o 1048576
	@sudo mkdosfs -F32 -f 2 /dev/loop1
	@sudo mount /dev/loop1 /mnt/fatgrub
	@sudo grub-install --root-directory=/mnt/fatgrub --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop0
	@sudo cp -r build/isofiles/* /mnt/fatgrub
	@sudo umount /mnt/fatgrub
	@losetup -d /dev/loop0


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
