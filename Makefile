export PATH := $(abspath tools/x86_64-elf-cross/bin):$(PATH)

OS_NAME = MidnightOS

BUILD_DIR = $(abspath build)
SOURCE_DIR = $(abspath src)
DATA_DIR = $(abspath files)

OVMF_BINARIES_DIR = ovmf-bins
GNU_EFI_DIR = gnu-efi

EMU = qemu-system-x86_64
DBG = gdb
CC = x86_64-elf-gcc
AC = nasm
LD = x86_64-elf-ld

DBG_BUILD = true

EMU_BASE_FLAGS = -drive file=$(BUILD_DIR)/$(OS_NAME).img,format=raw \
				-m 2G \
				-cpu qemu64 \
				-vga std \
				-drive if=pflash,format=raw,unit=0,file="$(OVMF_BINARIES_DIR)/OVMF_CODE-pure-efi.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="$(OVMF_BINARIES_DIR)/OVMF_VARS-pure-efi.fd" \
				-net none \
				-machine q35

EMU_DBG_FLAGS = -s -S -d guest_errors,cpu_reset,int -no-reboot -no-shutdown

DBG_FLAGS = -ex "target remote localhost:1234" \
			-ex "symbol-file $(BUILD_DIR)/kernel/kernel.elf" \
			-ex "set disassemble-next-line on" \
			-ex "set step-mode on"


C_DBG_DEFS = -D_DEBUG

C_F_FLAGS = -ffreestanding -fshort-wchar -fno-omit-frame-pointer -fno-builtin -fno-stack-protector \
			-fno-exceptions -fno-tree-vectorize -fno-builtin-memcpy -fno-builtin-memset

C_W_FLALGS = -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wundef \
			 -Wcast-align -Wshift-overflow -Wdouble-promotion -Werror

C_OP_LVL = -O0

CFLAGS = -g -mno-red-zone -m64 -mcmodel=kernel -nostartfiles -nodefaultlibs -nostdlib $(C_W_FLALGS) $(C_F_FLAGS)

ifeq ($(DBG_BUILD),true)
	CFLAGS += $(C_DBG_DEFS)
endif

CPPFLAGS = $(CFLAGS) -fno-rtti -fno-use-cxa-atexit -std=c++20


ACFLAGS = -f elf64

LDFLAGS = -static -Bsymbolic -nostdlib

partial: build update-img

all: init-img build-gnu-efi partial

build: build-bootloader build-executables

build-gnu-efi:
	$(MAKE) -C $(GNU_EFI_DIR) all

build-bootloader:
	@echo "\e[1;32m\n_____BUILDING_BOOTLOADER_____\e[0m"
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(SOURCE_DIR)/bootloader BUILD_DIR="$(BUILD_DIR)/bootloader" all

build-executables:
	@echo "\e[1;32m\n_____BUILDING_EXECUTABLES_____\e[0m"
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(SOURCE_DIR) BUILD_DIR="$(BUILD_DIR)" \
							FILES_DIR="$(DATA_DIR)" \
							CC="$(CC)" \
							LD="$(LD)" \
							AC="$(AC)" \
							CFLAGS="$(CFLAGS)" \
							CPPFLAGS="$(CPPFLAGS)" \
							LDFLAGS="$(LDFLAGS)" \
							ACFLAGS="$(ACFLAGS)" \
							all

update-img:
	@echo "\e[1;32m\n_____BUILDING_IMAGE_____\e[0m"
	mformat -i $(BUILD_DIR)/$(OS_NAME).img -F ::
	mmd -i $(BUILD_DIR)/$(OS_NAME).img ::/EFI
	mmd -i $(BUILD_DIR)/$(OS_NAME).img ::/EFI/BOOT
	mcopy -i $(BUILD_DIR)/$(OS_NAME).img $(BUILD_DIR)/bootloader/bootx64.efi ::/EFI/BOOT
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
	find $(SOURCE_DIR) -name "*.so" -type f -delete
	find $(BUILD_DIR) -name "*.o" -type f -delete
	find $(BUILD_DIR) -name "*.so" -type f -delete
	find $(BUILD_DIR) -name "*.efi" -type f -delete
	find $(BUILD_DIR) -name "*.efi.debug" -type f -delete
	find $(BUILD_DIR) -name "*.elf" -type f -delete