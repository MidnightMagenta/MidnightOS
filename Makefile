export TOPLEVEL_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
export MAKE_VARS := $(abspath vars.mk)
export MAKE_TARGETS := $(abspath targets.mk)

include $(MAKE_VARS)
include $(MAKE_TARGETS)
include $(TARGETS)

kernel_objs := $(patsubst %.o,$(BUILD_DIR)/%.o,$(obj-y))
#TODO: implement module building

.PHONY: rebuild all clean clean-all image run run-extra debug bootloader

.NOTPARALLEL: rebuild

all: $(BUILD_DIR)/$(KERNEL_TARGET) bootloader

rebuild: clean all

bootloader: $(GNU_EFI_BUILT_NOTE)
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C bootloader all

# debug rules
image: $(IMAGE) all
	@mkdir -p $(FILES_DIR)/EFI/BOOT $(FILES_DIR)/BOOT $(FILES_DIR)/MdOS/bin 
	@cp $(BOOTX64_BUILD_TARGET) $(FILES_DIR)/EFI/BOOT
	@cp $(MDOSBOOT_BUILD_TAGET) $(FILES_DIR)/BOOT
	@cp $(BUILD_DIR)/$(KERNEL_TARGET) $(FILES_DIR)/MdOS/bin
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

$(IMAGE):
	@mkdir -p $(BUILD_DIR)
	@dd if=/dev/zero of=$(IMAGE) bs=512 count=93750
	@sgdisk -s $(IMAGE) --disk-guid=$(DISK_GUID)
	@sgdisk -s $(IMAGE) --largest-new=1 --typecode=1:ef00 --partition-guid=1:$(ESP_GUID)

# build rules
$(GNU_EFI_BUILT_NOTE):
	@$(MAKE) -C $(GNU_EFI_DIR) all
	@touch $@

$(BUILD_DIR)/$(KERNEL_TARGET): $(kernel_objs)
	@echo "\e[1;32mLinking:\e[0m $@"
	@$(LD) $(LDFLAGS) -T linker/kernel_linker.ld -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling: $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.asm
	@echo "Assembling: $<"
	@mkdir -p $(@D)
	@$(AC) $(ACFLAGS) -o $@ $<