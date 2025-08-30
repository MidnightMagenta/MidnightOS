include $(MAKE_VARS)

.PHONY: all build_kernel

all: build_kernel

build_kernel:
	@$(MAKE) -C $(SOURCE_DIR)/bootx64 BUILD_DIR="$(BUILD_DIR)/bootx64" all
	@$(MAKE) -C $(SOURCE_DIR)/bootloader BUILD_DIR="$(BUILD_DIR)/bootloader" all
	@$(MAKE) -C $(SOURCE_DIR)/kernel all