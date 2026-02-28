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
CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
          -Ikernel/include -MMD -MP
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
GRUB_CFG   = grub/grub.cfg

# ========================
# Source discovery
# ========================
C_SRCS   := $(wildcard kernel/*.c)
ASM_SRCS := $(wildcard kernel/*.s)

OBJS := \
	$(C_SRCS:kernel/%.c=$(BUILD)/%.o) \
	$(ASM_SRCS:kernel/%.s=$(BUILD)/%.o)

DEPS := $(OBJS:.o=.d)

# ========================
# Default target
# ========================
all: run

# ========================
# Compile C sources
# ========================
$(BUILD)/%.o: kernel/%.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# ========================
# Assemble ASM sources
# ========================
$(BUILD)/%.o: kernel/%.s
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
$(KERNEL_BIN): $(KERNEL_ELF) $(GRUB_CFG)
	mkdir -p $(GRUB)
	cp $(KERNEL_ELF) $(BOOT)/kernel.bin
	cp $(GRUB_CFG) $(GRUB)/grub.cfg

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

# ========================
# Dependency tracking
# ========================
-include $(DEPS)

.PHONY: all run clean
