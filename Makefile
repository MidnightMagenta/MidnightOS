export TOPLEVEL_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

ARCH := x86_64
BIN_TARGET := elf
PREFIX := $(ARCH)-$(BIN_TARGET)-

CC := $(PREFIX)gcc
LD := $(PREFIX)ld
AR := $(PREFIX)ar
AC := $(PREFIX)as

DEBUG := true
DEBUGER := true
VERBOSE := false
OPTIMIZE := -O0

BUILD_DIR := build/$(ARCH)
LIB_DIR := $(BUILD_DIR)/libs

CFLAGS := -nostartfiles \
			-nodefaultlibs \
			-nostdlib \
			-nostdinc \
			-ffreestanding \
			-fshort-wchar \
			-fno-omit-frame-pointer \
			-fno-stack-protector \
		 	-fno-builtin \
			-fno-tree-vectorize \
			-fno-pic -fno-pie \
			-I./arch/$(ARCH)/include \
			-I./include \
			-std=gnu23 \
			-MMD -MP \
			$(OPTIMIZE)
LDFLAGS := -static -Bsymbolic -nostdlib -L$(LIB_DIR)
ACFLAGS := 

ifeq ($(ARCH),x86_64)
    AC := nasm
    ACFLAGS := -f elf64 -i$(abspath ./include) -i$(abspath ./arch/$(ARCH)/include)
    CFLAGS += -m64 -m80387 -msse -msse2 -mmmx \
				-mno-sse3 -mno-sse4 -mno-avx -mno-avx2 -mno-avx512f \
				-mcmodel=kernel -mno-red-zone
else
    $(error Unsuported architecture $(ARCH))
    # TODO: implement other arches
endif

ifeq ($(DEBUG),true)
    CFLAGS += -Wall -Wextra -g -D_DEBUG
    ACFLAGS += -g -d_DEBUG
    ifeq ($(DEBUGER), true)
        CFLAGS += -D_DEBUGER_START
    endif
endif

ifeq ($(VERBOSE),true)
    CFLAGS += -Wconversion -Wsign-conversion -Wundef -Wcast-align -Wshift-overflow \
	  			-Wdouble-promotion -Wpedantic -Werror
endif

GNU_EFI_DIR := bootloader/gnu-efi
GNU_EFI_NOTE := $(BUILD_DIR)/.gnu_efi_built

KERNEL_TARGET := nyxos.elf
KLIB_TARGET := libnyx.a

obj-y := arch/$(ARCH)/ debug/ mm/
lib-y := lib/
# build rules

.PHONY: all rebuild rebuild-all kernel bootloader clean clean-all image ccdb
.NOTPARALLEL: rebuild rebuild-all

all: kernel bootloader

kernel: $(BUILD_DIR)/$(KERNEL_TARGET)

rebuild: clean all
rebuild-all: clean-all all

BOOT_TYPE := uefi

bootloader: $(GNU_EFI_NOTE)
	@$(MAKE) -C bootloader BUILD_DIR="$(abspath $(BUILD_DIR)/bootloader)" INCLUDE_DIR="$(abspath ./include/)" BOOT_TYPE="$(BOOT_TYPE)" ARCH="$(ARCH)" all

$(GNU_EFI_NOTE):
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C $(GNU_EFI_DIR) all
	@touch $@

include scripts/build.mk

# TODO: add kernel module build rules

clean-all: clean
	@$(MAKE) -C $(GNU_EFI_DIR) clean
	@rm -rf $(GNU_EFI_NOTE)
	@rm -rf $(BUILD_DIR)

clean:
	@make -C bootloader BUILD_DIR="$(abspath $(BUILD_DIR)/bootloader)" \
	INCLUDE_DIR="$(abspath ./include/)" BOOT_TYPE="uefi" ARCH="$(ARCH)" clean
	@find $(BUILD_DIR) -name "*.o" -type f -delete
	@find $(BUILD_DIR) -name "*.a" -type f -delete
	@find $(BUILD_DIR) -name "*.d" -type f -delete
	@find $(BUILD_DIR) -name "*.so" -type f -delete
	@find $(BUILD_DIR) -name "*.efi" -type f -delete
	@find $(BUILD_DIR) -name "*.EFI" -type f -delete
	@find $(BUILD_DIR) -name "*.efi.debug" -type f -delete
	@find $(BUILD_DIR) -name "*.elf" -type f -delete

# image building rules

FILES_DIR := files
IMAGE := $(BUILD_DIR)/nyxos.img
DISK_GUID = f953b4de-e77f-4f0b-a14e-2b29080599cf
ESP_GUID = 0cc13370-53ec-4cdb-8c3d-4185950e2581

image: $(IMAGE)
	@mkdir -p $(FILES_DIR)/BOOT
	@sh ./scripts/genbootcfg.sh "$(FILES_DIR)/BOOT/BOOT.CFG" "$(ESP_GUID)" "$(KERNEL_TARGET)"
	@sudo sh ./scripts/updateimg.sh "$(IMAGE)" "$(BUILD_DIR)" "$(FILES_DIR)"

$(IMAGE):
	@echo -e "\e[1;32mCreating empty image\e[0m"
	@mkdir -p $(@D)
	@dd if=/dev/zero of=$@ bs=512 count=93750
	@sgdisk -s $(IMAGE) --disk-guid=$(DISK_GUID)
	@sgdisk -s $(IMAGE) --largest-new=1 --typecode=1:ef00 --partition-guid=1:$(ESP_GUID)

include scripts/run.mk

# misc

ccdb:
	@compiledb make -Bn all
