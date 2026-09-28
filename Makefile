CC ?= clang
CFLAGS ?= -O2 -Wall -Wextra -std=gnu11
KERNEL_CFLAGS := $(CFLAGS) -ffreestanding -fno-stack-protector -fno-stack-check -fno-pie -fno-pic -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mcmodel=kernel -I./src
KERNEL_LDFLAGS := -nostdlib -fuse-ld=lld -Wl,-T,linker.ld -Wl,-z,max-page-size=0x1000 -Wl,--gc-sections -no-pie

KERNEL := build/myos.elf
ISO := build/myos.iso
LIMINE_DIR := limine-binary

.PHONY: all clean run limine

all: $(ISO)

build:
	mkdir -p build

limine:
	@if [ ! -x $(LIMINE_DIR)/limine ]; then \
		rm -rf $(LIMINE_DIR); \
		curl -L https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz | tar -xz; \
		$(MAKE) -C $(LIMINE_DIR) CC="$(CC)"; \
	fi

$(KERNEL): build src/kernel.c src/limine.h linker.ld
	$(CC) $(KERNEL_CFLAGS) -c src/kernel.c -o build/kernel.o
	$(CC) $(KERNEL_LDFLAGS) build/kernel.o -o $(KERNEL)

$(ISO): $(KERNEL) limine
	rm -rf build/iso_root
	mkdir -p build/iso_root/boot/limine build/iso_root/EFI/BOOT
	cp $(KERNEL) build/iso_root/boot/myos
	cp limine.conf build/iso_root/boot/limine/
	cp $(LIMINE_DIR)/limine-bios.sys $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin build/iso_root/boot/limine/
	cp $(LIMINE_DIR)/BOOTX64.EFI build/iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label build/iso_root -o $(ISO)
	$(LIMINE_DIR)/limine bios-install $(ISO)
	@echo "Built $(ISO)"

run: $(ISO)
	qemu-system-x86_64 -M q35 -cdrom $(ISO) -m 512M

clean:
	rm -rf build $(LIMINE_DIR)
