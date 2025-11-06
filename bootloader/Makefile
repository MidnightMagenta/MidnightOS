BUILD_DIR := ./build
BOOT_TYPE := uefi
ARCH := x86_64

.PHONY: all

all:
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C $(BOOT_TYPE)-$(ARCH)/bootx64 BUILD_DIR="$(BUILD_DIR)" all
	@$(MAKE) -C $(BOOT_TYPE)-$(ARCH)/nyxboot BUILD_DIR="$(BUILD_DIR)" all

clean:
	@find $(BOOT_TYPE)-$(ARCH) -name "*.o" -type f -delete
	@find $(BOOT_TYPE)-$(ARCH) -name "*.so" -type f -delete
	@find $(BUILD_DIR) -name "*.o" -type f -delete
	@find $(BUILD_DIR) -name "*.so" -type f -delete
	@find $(BUILD_DIR) -name "*.efi" -type f -delete
	@find $(BUILD_DIR) -name "*.EFI" -type f -delete
	@find $(BUILD_DIR) -name "*.efi.debug" -type f -delete
