include $(MAKE_VARS)

.PHONY: all bootloader kernel

all: bootloader kernel

bootloader:
	@$(MAKE) -C bootloader all
	
kernel:
	@$(MAKE) -C kernel all
	
	