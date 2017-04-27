arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
CC=~/opt/cross64/x86_64-elf-4.9.1-Linux-x86_64/bin/x86_64-elf-gcc

linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg
assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
	build/arch/$(arch)/%.o, $(assembly_source_files))
c_source_files := $(wildcard src/arch/$(arch)/*.c)
c_object_files := $(patsubst src/arch/$(arch)/%.c, \
	build/arch/$(arch)/%.o, $(c_source_files))
symbol_link_file := symbol.bin

.PHONY: all clean run iso

all: $(kernel)

clean:
	@rm -rf build
	-rm $(symbol_link_file)

run: $(iso)
	@qemu-system-x86_64 -monitor stdio -s -cdrom $(iso)

iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

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
	@$(CC) -c -g $< -o $@ -Wall -Werror
