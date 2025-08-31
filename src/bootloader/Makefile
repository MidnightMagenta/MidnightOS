include $(MAKE_VARS)

.PHONY: all

all:
	@$(MAKE) -C bootx64 BUILD_DIR="$(BUILD_DIR)/bootloader/bootx64" all
	@$(MAKE) -C mdosboot BUILD_DIR="$(BUILD_DIR)/bootloader/mdosboot" all
