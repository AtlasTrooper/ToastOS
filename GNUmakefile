.SUFFIXES:

# ========================
# Toolchain
# ========================
CC   = x86_64-elf-gcc
LD   = x86_64-elf-ld
NASM = nasm
QEMU = qemu-system-x86_64

# ========================
# Flags
# ========================
CFLAGS = -std=gnu11 -ffreestanding -O2 -Wall -Wextra \
	-fno-stack-protector -fno-stack-check -fno-lto -fno-PIC \
	-ffunction-sections -fdata-sections \
	-m64 -march=x86-64 -mabi=sysv \
	-mno-80387 -mno-mmx -mno-sse -mno-sse2 \
	-mno-red-zone -mcmodel=kernel

CPPFLAGS = -Ikernel -MMD -MP

NASMFLAGS = -f elf64 -g -F dwarf -Wall

LDFLAGS = -m elf_x86_64 -nostdlib -static \
	-z max-page-size=0x1000 --gc-sections \
	-T linker.ld

QEMU_FLAGS = -serial stdio -m 1G
OVMF       = /usr/share/edk2/x64/OVMF.4m.fd

BOCHS      = bochs
BOCHSRC    = bochsrc.txt

# ========================
# Directories / outputs / hardware
# ========================
BUILD      = build
ISO_DIR    = isodir
KERNEL_ELF = $(BUILD)/kernel.elf
ISO        = $(BUILD)/toast.iso
DISK_IMG   = $(BUILD)/disk.img

USB_ID    ?= usb-SanDisk_Cruzer_Blade_04014828031623094824-0:0
USB_DEV   := /dev/disk/by-id/$(USB_ID)

# ========================
# Source discovery
# ========================
C_SRCS    := $(shell find kernel -type f -name '*.c' | LC_ALL=C sort)
# dlmalloc.c is #included directly by heap.c as a single translation unit
# so the dlmalloc_config.h defines are in scope before any dlmalloc code.
# Compiling it standalone would miss those defines and crash my shit.
C_SRCS    := $(filter-out kernel/mmu/dlmalloc.c, $(C_SRCS))
ASM_SRCS  := $(shell find kernel -type f \( -name '*.s' -o -name '*.S' \) | LC_ALL=C sort)
NASM_SRCS := $(shell find kernel -type f -name '*.asm' | LC_ALL=C sort)
PSF_SRCS  := $(shell find kernel -type f -name '*.psf' | LC_ALL=C sort)

OBJS := $(patsubst kernel/%.c,   $(BUILD)/%.c.o,   $(C_SRCS))   \
        $(patsubst kernel/%.s,   $(BUILD)/%.s.o,   $(filter %.s,  $(ASM_SRCS))) \
        $(patsubst kernel/%.S,   $(BUILD)/%.S.o,   $(filter %.S,  $(ASM_SRCS))) \
        $(patsubst kernel/%.asm, $(BUILD)/%.asm.o, $(NASM_SRCS)) \
        $(patsubst kernel/%.psf, $(BUILD)/%.psf.o, $(PSF_SRCS))

DEPS := $(OBJS:.o=.d)

# ========================
# Default target
# ========================
.PHONY: all
all: run

# ========================
# Compile C
# ========================
$(BUILD)/%.c.o: kernel/%.c GNUmakefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# ========================
# Assemble GAS (.S)
# ========================
$(BUILD)/%.s.o: kernel/%.s GNUmakefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.S.o: kernel/%.S GNUmakefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# ========================
# Assemble NASM (.asm)
# ========================
$(BUILD)/%.asm.o: kernel/%.asm GNUmakefile
	mkdir -p $(dir $@)
	$(NASM) $(NASMFLAGS) $< -o $@

# ========================
# PC Screen fonts (.psf)
# ========================
$(BUILD)/%.psf.o: kernel/%.psf GNUmakefile
	mkdir -p $(dir $@)
	objcopy -O elf64-x86-64 -B i386 -I binary $< $@

# ========================
# Link kernel
# ========================
$(KERNEL_ELF): GNUmakefile linker.ld $(OBJS)
	mkdir -p $(BUILD)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# ========================
# Build ISO (Limine CD-ROM)
# ========================
$(ISO): $(KERNEL_ELF)
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp $(KERNEL_ELF)             $(ISO_DIR)/kernel.elf
	cp limine.cfg                $(ISO_DIR)/
	cp limine/limine-bios.sys    $(ISO_DIR)/
	cp limine/limine-bios-cd.bin $(ISO_DIR)/
	cp limine/limine-uefi-cd.bin $(ISO_DIR)/
	cp limine/BOOTX64.EFI        $(ISO_DIR)/EFI/BOOT/
	cp limine/BOOTIA32.EFI       $(ISO_DIR)/EFI/BOOT/
	xorriso -as mkisofs \
		-b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		$(ISO_DIR) -o $(ISO)
	limine bios-install $(ISO)

.PHONY: iso
iso: $(ISO)

# ========================
# Build Raw GPT Disk Image (UEFI)
# ========================
$(DISK_IMG): $(KERNEL_ELF) limine.cfg
	@mkdir -p $(BUILD)
	@echo "==> Creating raw GPT disk image..."
	dd if=/dev/zero of=$@ bs=1M count=64 status=none
	parted -s $@ mklabel gpt
	parted -s $@ mkpart ESP fat32 2048s 100%
	parted -s $@ set 1 boot on
	@echo "==> Formatting (toast) and populating FAT32 partition..."
	@LOOP_DEV=$$(sudo losetup -Pf --show $@) ; \
	sudo mkfs.vfat -F 32 -n toast $${LOOP_DEV}p1 > /dev/null ; \
	mkdir -p $(BUILD)/mnt ; \
	sudo mount $${LOOP_DEV}p1 $(BUILD)/mnt ; \
	sudo mkdir -p $(BUILD)/mnt/EFI/BOOT ; \
	sudo cp $(KERNEL_ELF)             $(BUILD)/mnt/kernel.elf ; \
	sudo cp limine.cfg                $(BUILD)/mnt/ ; \
	sudo cp limine/BOOTX64.EFI        $(BUILD)/mnt/EFI/BOOT/ ; \
	sudo umount $(BUILD)/mnt ; \
	sudo losetup -d $${LOOP_DEV}

.PHONY: image
image: $(DISK_IMG)

# ========================
# Flash to Hardware USB
# ========================
.PHONY: flash
flash: $(DISK_IMG)
	@if [ ! -b $(USB_DEV) ]; then \
		echo "Error: USB drive ($(USB_DEV)) not found. Is it plugged in?"; \
		exit 1; \
	fi
	@echo "==> Unmounting any active partitions..."
	-sudo umount $$(readlink -f $(USB_DEV))* 2>/dev/null || true
	@echo "==> Flashing $(DISK_IMG) to $(USB_DEV)..."
	sudo dd if=$(DISK_IMG) of=$(USB_DEV) bs=4M status=progress conv=fsync
	sync
	@echo "==> Flash complete! Drive labeled toast."

# ========================
# Run in QEMU (BIOS / ISO)
# ========================
.PHONY: run
run: $(ISO)
	$(QEMU) -cdrom $(ISO) $(QEMU_FLAGS)

.PHONY: qemu-debug
qemu-debug: $(ISO)
	$(QEMU) -cdrom $(ISO) $(QEMU_FLAGS) -d int -no-reboot -no-shutdown

# ========================
# Run in QEMU (UEFI / GPT Disk)
# ========================
.PHONY: disk
disk: $(DISK_IMG)
	$(QEMU) -bios $(OVMF) -drive format=raw,file=$(DISK_IMG) $(QEMU_FLAGS)

.PHONY: disk-debug
disk-debug: $(DISK_IMG)
	$(QEMU) -bios $(OVMF) -drive format=raw,file=$(DISK_IMG) $(QEMU_FLAGS) -d int -no-reboot -no-shutdown

# ========================
# Bochs debugger
# ========================
.PHONY: bochs-debug
bochs-debug: $(ISO)
	$(BOCHS) -q -f $(BOCHSRC) -debugger

.PHONY: bochs
bochs: $(ISO)
	$(BOCHS) -q -f $(BOCHSRC)

# ========================
# Clean
# ========================
.PHONY: clean
clean:
	rm -rf $(BUILD) $(ISO_DIR)

-include $(DEPS)