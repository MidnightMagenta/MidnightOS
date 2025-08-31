include $(MAKE_VARS)

.PHONY: all build_kernel

all: build_kernel

build_kernel:
	@$(MAKE) -C bootloader all
	@$(MAKE) -C kernel all