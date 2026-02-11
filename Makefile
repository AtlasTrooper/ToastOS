# ========================
# Toolchain
# ========================
CC      = i686-elf-gcc
AS      = i686-elf-as
LD      = i686-elf-ld
QEMU    = qemu-system-i386

# ========================
# Flags
# ========================
CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib

# ========================
# Directories
# ========================
BUILD   = build
ISO_DIR = isodir
BOOT    = $(ISO_DIR)/boot
GRUB    = $(BOOT)/grub

# ========================
# Output files
# ========================
KERNEL_ELF = $(BUILD)/kernel.elf
KERNEL_BIN = $(BOOT)/kernel.bin
ISO        = $(BUILD)/toast.iso

# ========================
# Sources
# ========================
C_SRC   = kernel/kernel.c
ASM_SRC = kernel/boot.s

OBJS = \
	$(BUILD)/kernel.o \
	$(BUILD)/boot.o

# ========================
# Default target
# ========================
all: run

# ========================
# Compile C
# ========================
$(BUILD)/kernel.o: $(C_SRC)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# ========================
# Assemble boot code
# ========================
$(BUILD)/boot.o: $(ASM_SRC)
	mkdir -p $(BUILD)
	$(AS) $< -o $@

# ========================
# Link kernel
# ========================
$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# ========================
# Prepare ISO tree
# ========================
$(KERNEL_BIN): $(KERNEL_ELF) grub/grub.cfg
	mkdir -p $(GRUB)
	cp $(KERNEL_ELF) $(BOOT)/kernel.bin
	cp grub/grub.cfg $(GRUB)/grub.cfg

# ========================
# Build ISO
# ========================
$(ISO): $(KERNEL_BIN)
	grub-mkrescue -o $@ $(ISO_DIR)

# ========================
# Run in QEMU
# ========================
run: $(ISO)
	$(QEMU) -cdrom $<

# ========================
# Clean
# ========================
clean:
	rm -rf $(BUILD) $(ISO_DIR)

.PHONY: all run clean
