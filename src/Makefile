include $(MAKE_VARS)

all: build_monocypher build_kernel

build_monocypher:
	@echo "\e[1;32m\n_____BUILDING_MONOCYPHER_____\e[0m"
	$(MAKE) -C $(SOURCE_DIR)/monocypher all

build_kernel: 
	@echo "\e[1;32m\n_____BUILDING_KERNEL_____\e[0m"
	$(MAKE) -C $(SOURCE_DIR)/kernel	all