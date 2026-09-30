CC ?= clang
CFLAGS ?= -O2 -Wall -Wextra -std=gnu11
KERNEL_CFLAGS := $(CFLAGS) -ffreestanding -fno-stack-protector -fno-stack-check -fno-pie -fno-pic -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mcmodel=kernel -I./src
KERNEL_LDFLAGS := -nostdlib -fuse-ld=lld -Wl,-T,linker.ld -Wl,-z,max-page-size=0x1000 -Wl,--gc-sections -no-pie

KERNEL := build/myos.elf
ISO := build/myos.iso
NETWORK_OBJ := build/e1000.o
ATA_OBJ := build/ata_pio.o
VFS_OBJ := build/vfs.o
PE_OBJ := build/pe.o
PKG_OBJ := build/installer.o
MYX_OBJ := build/myx.o
MEM_OBJ := build/memory.o
PROC_OBJ := build/process.o
SYSCALL_OBJ := build/syscall.o
ELF_OBJ := build/elf_loader.o
LIMINE_DIR := limine-binary

.PHONY: all all-hdd clean run run-hdd limine

all: $(ISO)

HDD := build/myos.hdd

all-hdd: $(HDD)

$(HDD): $(KERNEL) limine tools/init_vfs_image.py
	rm -f $(HDD)
	dd if=/dev/zero of=$(HDD) bs=1M count=64
	sgdisk --zap-all $(HDD)
	sgdisk $(HDD) -n 1:2048:2111 -t 1:ef02 -c 1:MYOSBIOS
	sgdisk $(HDD) -n 2:4096:65535 -t 2:ef00 -c 2:MYOSBOOT
	sgdisk $(HDD) -n 3:65536:0 -t 3:8300 -c 3:MYOSDATA
	$(LIMINE_DIR)/limine bios-install $(HDD) 1
	mformat -i $(HDD)@@2097152 -T 61440 -h 255 -s 63 ::
	mmd -i $(HDD)@@2097152 ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i $(HDD)@@2097152 $(KERNEL) ::/boot/myos
	mcopy -i $(HDD)@@2097152 limine.conf ::/boot/limine
	mcopy -i $(HDD)@@2097152 $(LIMINE_DIR)/limine-bios.sys ::/boot/limine
	mcopy -i $(HDD)@@2097152 $(LIMINE_DIR)/BOOTX64.EFI ::/EFI/BOOT
	python3 tools/init_vfs_image.py $(HDD)
	@echo "Built $(HDD) with boot and persistent myVFS partitions"

build:
	mkdir -p build

limine:
	@if [ ! -x $(LIMINE_DIR)/limine ]; then 		rm -rf $(LIMINE_DIR); 		curl -L https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz | tar -xz; 		$(MAKE) -C $(LIMINE_DIR) CC="$(CC)"; 	fi

$(KERNEL): build src/kernel.c src/limine.h linker.ld $(NETWORK_OBJ) $(ATA_OBJ) $(VFS_OBJ) $(PE_OBJ) $(PKG_OBJ) $(MYX_OBJ) $(MEM_OBJ) $(PROC_OBJ) $(SYSCALL_OBJ) $(ELF_OBJ)
	$(CC) $(KERNEL_CFLAGS) -c src/kernel.c -o build/kernel.o
	$(CC) $(KERNEL_LDFLAGS) build/kernel.o $(NETWORK_OBJ) $(ATA_OBJ) $(VFS_OBJ) $(PE_OBJ) $(PKG_OBJ) $(MYX_OBJ) $(MEM_OBJ) $(PROC_OBJ) $(SYSCALL_OBJ) $(ELF_OBJ) -o $(KERNEL)

$(NETWORK_OBJ): kernel/net/e1000.c kernel/net/e1000.h
	$(CC) $(KERNEL_CFLAGS) -Ikernel/net -c kernel/net/e1000.c -o $(NETWORK_OBJ)

$(ATA_OBJ): kernel/storage/ata_pio.c kernel/storage/ata_pio.h
	$(CC) $(KERNEL_CFLAGS) -Ikernel/storage -c kernel/storage/ata_pio.c -o $(ATA_OBJ)

$(VFS_OBJ): kernel/fs/vfs.c kernel/fs/vfs.h $(ATA_OBJ)
	$(CC) $(KERNEL_CFLAGS) -Ikernel/fs -Ikernel/storage -c kernel/fs/vfs.c -o $(VFS_OBJ)

$(PE_OBJ): system/mywin/pe.c system/mywin/pe.h
	$(CC) $(KERNEL_CFLAGS) -Isystem/mywin -c system/mywin/pe.c -o $(PE_OBJ)

$(PKG_OBJ): system/mypkg/installer.c system/mypkg/installer.h kernel/fs/vfs.h
	$(CC) $(KERNEL_CFLAGS) -Isystem/mypkg -Ikernel/fs -c system/mypkg/installer.c -o $(PKG_OBJ)

$(MYX_OBJ): system/runtime/myx.c system/runtime/myx.h
	$(CC) $(KERNEL_CFLAGS) -Isystem/runtime -c system/runtime/myx.c -o $(MYX_OBJ)

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

run-hdd: $(HDD)
	qemu-system-x86_64 -M pc -hda $(HDD) -m 512M

clean:
	rm -rf build $(LIMINE_DIR)

$(MEM_OBJ): kernel/memory/memory.c kernel/memory/memory.h src/limine.h
	$(CC) $(KERNEL_CFLAGS) -Ikernel/memory -c kernel/memory/memory.c -o $(MEM_OBJ)

$(PROC_OBJ): kernel/process/process.c kernel/process/process.h
	$(CC) $(KERNEL_CFLAGS) -Ikernel/process -c kernel/process/process.c -o $(PROC_OBJ)

$(SYSCALL_OBJ): system/runtime/syscall.c system/runtime/syscall.h kernel/process/process.h
	$(CC) $(KERNEL_CFLAGS) -Isystem/runtime -Ikernel/process -c system/runtime/syscall.c -o $(SYSCALL_OBJ)

$(ELF_OBJ): system/runtime/elf_loader.c system/runtime/elf_loader.h
	$(CC) $(KERNEL_CFLAGS) -Isystem/runtime -c system/runtime/elf_loader.c -o $(ELF_OBJ)
