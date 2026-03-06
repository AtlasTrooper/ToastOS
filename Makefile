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

QEMU_FLAGS = -serial stdio

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
C_SRCS   := $(shell find kernel -name "*.c")
ASM_SRCS := $(shell find kernel -name "*.s")

OBJS := $(patsubst kernel/%.c,$(BUILD)/%.o,$(C_SRCS)) \
        $(patsubst kernel/%.s,$(BUILD)/%.o,$(ASM_SRCS))

DEPS := $(OBJS:.o=.d)

# ========================
# Default target
# ========================
all: run

# ========================
# Compile C sources
# ========================
$(BUILD)/%.o: kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ========================
# Assemble ASM sources
# ========================
$(BUILD)/%.o: kernel/%.s
	mkdir -p $(dir $@)
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
	$(QEMU) -cdrom $< $(QEMU_FLAGS)

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
