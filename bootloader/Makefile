BUILD_DIR := .
BOOT_TYPE := uefi
ARCH := x86_64

.PHONY: all

all:
	@$(MAKE) -C $(BOOT_TYPE)-$(ARCH)/bootx64 BUILD_DIR="$(BUILD_DIR)" all
	@$(MAKE) -C $(BOOT_TYPE)-$(ARCH)/mdosboot BUILD_DIR="$(BUILD_DIR)" all
