export TOPLEVEL_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

ARCH := x86_64
BIN_TARGET := elf
PREFIX := $(ARCH)-$(BIN_TARGET)-

CC := $(PREFIX)gcc
LD := $(PREFIX)ld
AC :=

DEBUG := true
VERBOSE := false
OPTIMIZE := -O0

CFLAGS := -nostartfiles \
					-nodefaultlibs \
					-nostdlib \
					-ffreestanding \
					-fshort-wchar \
					-fno-omit-frame-pointer \
					-fno-stack-protector \
		 			-fno-builtin \
					-fno-tree-vectorize \
					-fno-pic -fno-pie \
					-I./include \
					-std=gnu17 \
					-MMD -MP \
					$(OPTIMIZE)
LDFLAGS := -static -Bsymbolic -nostdlib
ACFLAGS :=

ifeq ($(ARCH),x86_64)
  AC := nasm
  ACFLAGS := -f elf64
  CFLAGS += -m64 -m80387 -msse -msse2 -mmmx \
						-mno-sse3 -mno-sse4 -mno-avx -mno-avx2 -mno-avx512f \
						-mcmodel=kernel -mno-red-zone
else
  $(error Unsuported architecture $(ARCH))
  # TODO: implement other arches
endif

ifeq ($(DEBUG),true)
  CFLAGS += -Wall -Wextra -g -D_DEBUG
  ACFLAGS += -g
endif

ifeq ($(VERBOSE),true)
  CFLAGS += -Wconversion -Wsign-conversion -Wundef -Wcast-align -Wshift-overflow \
	  				-Wdouble-promotion -Wpedantic -Werror
endif

BUILD_DIR := build/$(ARCH)

GNU_EFI_DIR := gnu-efi
GNU_EFI_NOTE := $(BUILD_DIR)/.gnu_efi_built

KERNEL_TARGET := mdoskrnl.elf

obj-y :=

# include source directories
-include arch/$(ARCH)/config.mk
-include debug/config.mk

# build rules
KERNEL_OBJS := $(patsubst %.o,$(BUILD_DIR)/%.o,$(obj-y))
DEP_OBJS := $(KERNEL_OBJS:.o=.d)

.PHONY: all rebuild rebuild-all bootloader clean clean-all image run run-info run-debug ccdb
.NOTPARALLEL: rebuild rebuild-all

all: bootloader $(BUILD_DIR)/$(KERNEL_TARGET)

rebuild: clean all
rebuild-all: clean-all all

bootloader: $(GNU_EFI_NOTE)
	@$(MAKE) -C bootloader BUILD_DIR="$(abspath $(BUILD_DIR)/bootloader)" INCLUDE_DIR="$(abspath ./include)" all

$(GNU_EFI_NOTE):
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C $(GNU_EFI_DIR) all
	@touch $@

$(BUILD_DIR)/$(KERNEL_TARGET): $(KERNEL_OBJS)
	@echo -e "\e[1;32mLinking:\e[0m $@"
	@$(LD) $(LDFLAGS) -T link/$(ARCH)-link.ld -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@echo -e "Compiling: $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.asm
	@echo -e "Assembling: $<"
	@mkdir -p $(@D)
	$(AC) $(ACFLAGS) -o $@ $<

-include $(DEP_OBJS)

# TODO: add kernel module build rules

clean-all: clean
	@$(MAKE) -C $(GNU_EFI_DIR) clean
	@rm -rf $(GNU_EFI_NOTE)
	@rm -rf $(BUILD_DIR)

clean:
	@find bootloader -name "*.o" -type f -delete
	@find bootloader -name "*.so" -type f -delete
	@find $(BUILD_DIR) -name "*.o" -type f -delete
	@find $(BUILD_DIR) -name "*.a" -type f -delete
	@find $(BUILD_DIR) -name "*.so" -type f -delete
	@find $(BUILD_DIR) -name "*.efi" -type f -delete
	@find $(BUILD_DIR) -name "*.EFI" -type f -delete
	@find $(BUILD_DIR) -name "*.efi.debug" -type f -delete
	@find $(BUILD_DIR) -name "*.elf" -type f -delete

# image building rules

FILES_DIR := files
IMAGE := $(BUILD_DIR)/mdos.img
DISK_GUID = f953b4de-e77f-4f0b-a14e-2b29080599cf
ESP_GUID = 0cc13370-53ec-4cdb-8c3d-4185950e2581

image: $(IMAGE)
	@mkdir -p $(FILES_DIR)/BOOT
	@sh ./shell/genbootcfg.sh "$(FILES_DIR)/BOOT/BOOT.CFG" "$(ESP_GUID)" "MdOS\bin\$(KERNEL_TARGET)"
	@sudo sh ./shell/updateimg.sh "$(IMAGE)" "$(BUILD_DIR)" "$(FILES_DIR)"

$(IMAGE):
	@echo -e "\e[1;32mCreating empty image\e[0m"
	@mkdir -p $(@D)
	@dd if=/dev/zero of=$@ bs=512 count=93750
	@sgdisk -s $(IMAGE) --disk-guid=$(DISK_GUID)
	@sgdisk -s $(IMAGE) --largest-new=1 --typecode=1:ef00 --partition-guid=1:$(ESP_GUID)

# debug emulator run rules

EMU := 
DBG := 

ifeq ($(ARCH),x86_64)
  EMU := qemu-system-x86_64
  DBG := gdb
else
  $(error Unsuported architecture $(ARCH))
  # TODO: implement other arches
endif

OVMF_BINS := ovmf-bins/$(ARCH)

EMU_BASE_FLAGS = -drive file=$(IMAGE),format=raw \
				-m 2G \
				-cpu qemu64 \
				-vga std \
				-drive if=pflash,format=raw,unit=0,file="$(OVMF_BINS)/OVMF_CODE-pure-efi.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="$(OVMF_BINS)/OVMF_VARS-pure-efi.fd" \
				-net none \
				-machine q35 

EMU_DBG_FLAGS = -s -S -d int,guest_errors,cpu_reset -no-reboot -no-shutdown -D tmp/qemu.log

DBG_FLAGS = -ex "symbol-file $(BUILD_DIR)/$(KERNEL_TARGET)" \
			-ex "target remote localhost:1234" \
			-ex "set disassemble-next-line on" \
			-ex "set step-mode on"

run:
	$(EMU) $(EMU_BASE_FLAGS)

run-info:
	@mkdir -p ./tmp
	$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS)

run-debug:
	@mkdir -p ./tmp
	@$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS) &
	@$(DBG) $(DBG_FLAGS)

# misc

ccdb:
	@compiledb make -Bn