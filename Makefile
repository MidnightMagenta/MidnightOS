export PATH := $(abspath tools/x86_64-elf-cross/bin):$(PATH)

OS_NAME = MidnightOS

BUILD_DIR = $(abspath build)
SOURCE_DIR = $(abspath src)
DATA_DIR = $(abspath files)

OVMF_BINARIES_DIR = ovmf-bins
GNU_EFI_DIR = gnu-efi

EFI_TARGET = bootx64.efi
KERNEL_ELF_TARGET = kernel.elf

EMU = qemu-system-x86_64
DBG = gdb
CC = x86_64-elf-g++
AC = nasm
LD = x86_64-elf-ld

EMU_BASE_FLAGS = -drive file=$(BUILD_DIR)/$(OS_NAME).img,format=raw \
				-m 2G \
				-cpu qemu64 \
				-vga std \
				-drive if=pflash,format=raw,unit=0,file="$(OVMF_BINARIES_DIR)/OVMF_CODE-pure-efi.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="$(OVMF_BINARIES_DIR)/OVMF_VARS-pure-efi.fd" \
				-net none \
				-machine q35

EMU_DBG_FLAGS = -s -d guest_errors,cpu_reset,int -no-reboot -no-shutdown

DBG_FLAGS = -ex "target remote localhost:1234" \
			-ex "symbol-file $(BUILD_DIR)/kernel/$(ELF_TARGET)" \
			-ex "set disassemble-next-line on" \
			-ex "set step-mode on"

CFLAGS = -g -ffreestanding -fshort-wchar -mno-red-zone -m64 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion \
		-Wundef -Wcast-align -Wshift-overflow -Wdouble-promotion -nostdlib -fno-rtti -mcmodel=kernel \
		-fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -fno-exceptions -fno-use-cxa-atexit -O0 -Werror \

ACFLAGS = -f elf64

LDFLAGS = -static -Bsymbolic -nostdlib

partial: build update-img

all: init-img build-gnu-efi partial

build: build-bootloader build-kernel

build-gnu-efi:
	$(MAKE) -C $(GNU_EFI_DIR) all

build-bootloader:
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(SOURCE_DIR)/bootloader EFI_TARGET="$(EFI_TARGET)" BUILD_DIR="$(BUILD_DIR)/bootloader" all
	$(MAKE) -C $(SOURCE_DIR)/bootloader EFI_TARGET="$(EFI_TARGET).debug" BUILD_DIR="$(BUILD_DIR)/bootloader" all

build-kernel:
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(SOURCE_DIR)/kernel ELF_TARGET="$(KERNEL_ELF_TARGET)" \
									BUILD_DIR="$(BUILD_DIR)/kernel" \
									CC="$(CC)" \
									LD="$(LD)" \
									AC="$(AC)" \
									CFLAGS="$(CFLAGS)" \
									LDFLAGS="$(LDFLAGS)" \
									ACFLAGS="$(ACFLAGS)" \
									all

update-img:
	mformat -i $(BUILD_DIR)/$(OS_NAME).img -F ::
	mmd -i $(BUILD_DIR)/$(OS_NAME).img ::/EFI
	mmd -i $(BUILD_DIR)/$(OS_NAME).img ::/EFI/BOOT
	mmd -i $(BUILD_DIR)/$(OS_NAME).img ::/BOOT
	mcopy -i $(BUILD_DIR)/$(OS_NAME).img $(BUILD_DIR)/bootloader/$(EFI_TARGET) ::/EFI/BOOT
	mcopy -i $(BUILD_DIR)/$(OS_NAME).img $(BUILD_DIR)/kernel/$(KERNEL_ELF_TARGET) ::/BOOT
	mcopy -si $(BUILD_DIR)/$(OS_NAME).img $(DATA_DIR)/* ::

init-img:
	@mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$(BUILD_DIR)/$(OS_NAME).img bs=512 count=93750

run:
	$(EMU) $(EMU_BASE_FLAGS)

run-extra-info:
	$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS)

debug:
	$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS) &
	$(DBG) $(DBG_FLAGS)

clean-all: clean
	$(MAKE) -C gnu-efi clean
	rm -rf $(BUILD_DIR)

clean:
	find $(SOURCE_DIR) -name "*.o" -type f -delete
	find $(BUILD_DIR) -name "*.o" -type f -delete
	find $(BUILD_DIR) -name "*.so" -type f -delete
	find $(BUILD_DIR) -name "*.efi" -type f -delete
	find $(BUILD_DIR) -name "*.efi.debug" -type f -delete
	find $(BUILD_DIR) -name "*.elf" -type f -delete