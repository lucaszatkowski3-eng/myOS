CC ?= clang
LD ?= ld.lld
CFLAGS := -std=c11 -O2 -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-pic -mno-red-zone -mcmodel=kernel -mno-sse -mno-sse2 -mno-mmx -mno-80387 -I./src
LDFLAGS := -nostdlib -z max-page-size=0x1000 -T linker.ld

KERNEL := build/myos.elf
ISO := build/myos.iso

.PHONY: all clean run limine

all: $(ISO)

limine:
	@if [ ! -d limine ]; then git clone --depth 1 https://github.com/limine-bootloader/limine.git limine; fi
	@make -C limine

build:
	mkdir -p build

$(KERNEL): build src/kernel.c src/limine.h linker.ld
	$(CC) $(CFLAGS) -c src/kernel.c -o build/kernel.o
	$(LD) $(LDFLAGS) build/kernel.o -o $(KERNEL)

$(ISO): $(KERNEL) limine
	mkdir -p build/iso_root/boot/limine build/iso_root/EFI/BOOT
	cp $(KERNEL) build/iso_root/boot/myos
	cp limine.conf build/iso_root/boot/limine/limine.conf
	cp limine/limine-bios.sys build/iso_root/boot/limine/
	cp limine/limine-bios-cd.bin build/iso_root/boot/limine/
	cp limine/limine-uefi-cd.bin build/iso_root/boot/limine/
	cp limine/BOOTX64.EFI build/iso_root/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label build/iso_root -o $(ISO)
	limine bios-install $(ISO)
	@echo "Built $(ISO)"

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 512M

clean:
	rm -rf build
