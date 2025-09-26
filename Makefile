export TOPLEVEL_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export MAKE_VARS := $(abspath vars.mk)
export MAKE_TARGETS := $(abspath targets.mk)
include $(MAKE_VARS)
include $(MAKE_TARGETS)

GNU_EFI_BUILT_NOTE := $(BUILD_DIR)/.gnu-efi-note

EMU_BASE_FLAGS = -drive file=$(IMAGE),format=raw 																		\
				-m 2G 																									\
				-cpu qemu64 																							\
				-vga std																								\
				-drive if=pflash,format=raw,unit=0,file="$(OVMF_BINARIES_DIR)/OVMF_CODE-pure-efi.fd",readonly=on 		\
				-drive if=pflash,format=raw,unit=1,file="$(OVMF_BINARIES_DIR)/OVMF_VARS-pure-efi.fd" 					\
				-net none 																								\
				-machine q35

EMU_DBG_FLAGS = -s -S -d guest_errors,cpu_reset,int -no-reboot -no-shutdown

DBG_FLAGS = -ex "symbol-file $(BUILD_DIR)/kernel/kernel.elf" 															\
			-ex "target remote localhost:1234" 																			\
			-ex "set disassemble-next-line on" 																			\
			-ex "set step-mode on"

DISK_GUID = f953b4de-e77f-4f0b-a14e-2b29080599cf
ESP_GUID = 0cc13370-53ec-4cdb-8c3d-4185950e2581

.PHONY: rebuild all build clean clean-all image run run-extra debug bootlaoder kernel

.NOTPARALLEL: rebuild

all: image

rebuild: clean all

build: $(GNU_EFI_BUILT_NOTE)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)
	@$(MAKE) -C $(SOURCE_DIR) all

bootlaoder: $(GNU_EFI_BUILT_NOTE)
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C $(SOURCE_DIR) bootlaoder

kernel:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)
	@$(MAKE) -C $(SOURCE_DIR) kernel

image: $(IMAGE) build
	@mkdir -p $(FILES_DIR)/EFI/BOOT $(FILES_DIR)/BOOT $(FILES_DIR)/MdOS/bin 
	@cp $(KERNEL_BUILD_TARGET) $(FILES_DIR)/MdOS/bin
	@cp $(BOOTX64_BUILD_TARGET) $(FILES_DIR)/EFI/BOOT
	@cp $(MDOSBOOT_BUILD_TAGET) $(FILES_DIR)/BOOT
	@sudo sh $(UPDATEIMG_SH) "$(IMAGE)" "$(FILES_DIR)"

run:
	@$(EMU) $(EMU_BASE_FLAGS)

run-extra:
	@$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS)

debug:
	@$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS) &
	@$(DBG) $(DBG_FLAGS)

clean-all: clean
	@$(MAKE) -C $(GNU_EFI_DIR) clean
	@rm -rf $(GNU_EFI_BUILT_NOTE)
	@rm -rf $(BUILD_DIR)

clean:
	@find $(SOURCE_DIR) -name "*.o" -type f -delete
	@find $(SOURCE_DIR) -name "*.so" -type f -delete
	@find $(BUILD_DIR) -name "*.o" -type f -delete
	@find $(BUILD_DIR) -name "*.a" -type f -delete
	@find $(BUILD_DIR) -name "*.so" -type f -delete
	@find $(BUILD_DIR) -name "*.efi" -type f -delete
	@find $(BUILD_DIR) -name "*.efi.debug" -type f -delete
	@find $(BUILD_DIR) -name "*.elf" -type f -delete

$(GNU_EFI_BUILT_NOTE):
	@$(MAKE) -C $(GNU_EFI_DIR) all
	@touch $@

$(IMAGE):
	@mkdir -p $(BUILD_DIR)
	@dd if=/dev/zero of=$(IMAGE) bs=512 count=93750
	@sgdisk -s $(IMAGE) --disk-guid=$(DISK_GUID)
	@sgdisk -s $(IMAGE) --largest-new=1 --typecode=1:ef00 --partition-guid=1:$(ESP_GUID)