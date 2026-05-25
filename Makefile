# =============================================================================
# RanOS – Makefile
# Requires: i686-elf-gcc, nasm, qemu-system-i386
# Optional: grub-mkrescue, xorriso  (for 'make iso')
# =============================================================================

CC      := gcc
LD      := ld
AS      := nasm
OBJCOPY := objcopy

CFLAGS  := -m32 -march=i686 -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
            -fno-pic -fno-stack-protector -fno-exceptions                \
            -nostdlib -nodefaultlibs
ASFLAGS := -f elf32
LDFLAGS := -m elf_i386 -T link.ld

BUILD   := build
ISODIR  := isodir
KERNEL  := $(BUILD)/kernel.elf
ISO     := ranos.iso
DISK    := ranos.img

# --------------------------------------------------------------------------
# Source files
# --------------------------------------------------------------------------

SRCS_C  := kernel/kernel.c                 \
            kernel/gdt.c                   \
            kernel/idt.c                   \
            kernel/isr.c                   \
            drivers/vga.c                  \
            drivers/keyboard.c             \
            mm/pmm.c                       \
            mm/heap.c                      \
            fs/fs.c                        \
            shell/shell.c                  \
            libc/string.c

SRCS_ASM := boot/multiboot.asm             \
            kernel/isr.asm

OBJS    := $(SRCS_C:%.c=$(BUILD)/%.o)           \
           $(SRCS_ASM:%.asm=$(BUILD)/%.asm.o)

# --------------------------------------------------------------------------
# Default target: build the kernel ELF
# --------------------------------------------------------------------------

.PHONY: all clean iso run run-iso run-disk

all: $(KERNEL)

$(KERNEL): $(OBJS) link.ld | $(BUILD)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	@echo ""
	@echo "  Kernel built: $@"
	@echo "  Run with:     make run"

# --------------------------------------------------------------------------
# Compile C sources
# --------------------------------------------------------------------------

$(BUILD)/%.o: %.c | $(BUILD)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --------------------------------------------------------------------------
# Assemble ASM sources  (.asm -> .asm.o to avoid name collision with .c -> .o)
# --------------------------------------------------------------------------

$(BUILD)/%.asm.o: %.asm | $(BUILD)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

# --------------------------------------------------------------------------
# ISO image (requires grub-mkrescue + xorriso)
# --------------------------------------------------------------------------

iso: $(KERNEL)
	@mkdir -p $(ISODIR)/boot/grub
	cp $(KERNEL) $(ISODIR)/boot/kernel.elf
	@printf 'set timeout=2\nset default=0\n\nmenuentry "RanOS v0.1" {\n    multiboot /boot/kernel.elf\n    boot\n}\n' \
		> $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISODIR) 2>/dev/null
	@echo "  ISO built: $(ISO)"

# --------------------------------------------------------------------------
# Raw disk image (MBR bootloader + kernel, for use with custom boot.asm)
# Requires: nasm, dd, i686-elf-objcopy
# --------------------------------------------------------------------------

disk: $(KERNEL) boot/boot.asm
	nasm -f bin -o $(BUILD)/boot.bin boot/boot.asm
	$(OBJCOPY) -O binary $(KERNEL) $(BUILD)/kernel.bin
	dd if=/dev/zero    bs=1M  count=4  of=$(DISK) status=none
	dd if=$(BUILD)/boot.bin  conv=notrunc of=$(DISK) status=none
	dd if=$(BUILD)/kernel.bin conv=notrunc seek=1 bs=512 of=$(DISK) status=none
	@echo "  Disk image: $(DISK)"

# --------------------------------------------------------------------------
# Run targets
# --------------------------------------------------------------------------

# Fastest: GRUB multiboot direct kernel load (no ISO needed)
run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -m 64M -serial stdio \
	    -no-reboot -no-shutdown

# Bootable ISO via GRUB
run-iso: iso
	qemu-system-i386 -cdrom $(ISO) -m 64M -serial stdio \
	    -no-reboot -no-shutdown

# Raw disk image via MBR bootloader
run-disk: disk
	qemu-system-i386 -drive format=raw,file=$(DISK) -m 64M \
	    -no-reboot -no-shutdown

# --------------------------------------------------------------------------

clean:
	rm -rf $(BUILD) $(ISODIR) $(ISO) $(DISK)
