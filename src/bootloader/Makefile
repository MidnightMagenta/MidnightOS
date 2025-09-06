include $(MAKE_VARS)
include $(MAKE_TARGETS)

.PHONY: all

all:
	@$(MAKE) -C bootx64 BUILD_DIR="$(BOOTX64_BUILD_DIR)" all
	@$(MAKE) -C mdosboot BUILD_DIR="$(MDOSBOOT_BUILD_DIR)" all
