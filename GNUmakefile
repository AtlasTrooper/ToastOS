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

QEMU_FLAGS = -serial stdio -m 256M

# ========================
# Directories / outputs
# ========================
BUILD      = build
ISO_DIR    = isodir
KERNEL_ELF = $(BUILD)/kernel.elf
ISO        = $(BUILD)/toast.iso

# ========================
# Source discovery
# ========================
C_SRCS    := $(shell find kernel -type f -name '*.c' | LC_ALL=C sort)
ASM_SRCS  := $(shell find kernel -type f -name '*.s' | LC_ALL=C sort)
NASM_SRCS := $(shell find kernel -type f -name '*.asm' | LC_ALL=C sort)
PSF_SRCS  := $(shell find kernel -type f -name '*.psf' | LC_ALL=C sort)

OBJS := $(patsubst kernel/%.c,   $(BUILD)/%.c.o,   $(C_SRCS))   \
        $(patsubst kernel/%.s,   $(BUILD)/%.S.o,   $(ASM_SRCS)) \
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
$(BUILD)/%.S.o: kernel/%.s GNUmakefile
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
# Build ISO (Limine)
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
	./limine/limine bios-install $(ISO)

.PHONY: iso
iso: $(ISO)

# ========================
# Run in QEMU
# ========================
.PHONY: run
run: $(ISO)
	$(QEMU) -cdrom $(ISO) $(QEMU_FLAGS)

.PHONY: qemu-debug
qemu-debug: $(ISO)
	$(QEMU) -cdrom $(ISO) $(QEMU_FLAGS) -d int -no-reboot -no-shutdown

# ========================
# Clean
# ========================
.PHONY: clean
clean:
	rm -rf $(BUILD) $(ISO_DIR)

-include $(DEPS)
