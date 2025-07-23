export TOPLEVEL_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
export MAKE_VARS := $(abspath vars.mk)
include $(MAKE_VARS)

GNU_EFI_BUILT_NOTE := $(BUILD_DIR)/.gnu-efi-note

EMU_BASE_FLAGS = -drive file=$(IMAGE),format=raw \
				-m 2G \
				-cpu qemu64 \
				-vga std \
				-drive if=pflash,format=raw,unit=0,file="$(OVMF_BINARIES_DIR)/OVMF_CODE-pure-efi.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="$(OVMF_BINARIES_DIR)/OVMF_VARS-pure-efi.fd" \
				-net none \
				-machine q35

EMU_DBG_FLAGS = -s -S -d guest_errors,cpu_reset,int -no-reboot -no-shutdown

DBG_FLAGS = -ex "target remote localhost:1234" \
			-ex "symbol-file $(BUILD_DIR)/kernel/kernel.elf" \
			-ex "set disassemble-next-line on" \
			-ex "set step-mode on"

.PHONY: rebuild all build build-bootloader clean clean-all \
		build-executables update-img gen-keys run run-extra-info debug \

.NOTPARALLEL: rebuild

all: update-img

rebuild: clean all

build: build-bootloader build-executables

build-bootloader: $(GNU_EFI_BUILT_NOTE)
	@echo "\e[1;32m\n_____BUILDING_BOOTLOADER_____\e[0m"
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(SOURCE_DIR)/bootx64 BUILD_DIR="$(BUILD_DIR)/bootx64" all
	$(MAKE) -C $(SOURCE_DIR)/bootloader BUILD_DIR="$(BUILD_DIR)/bootloader" all

build-executables:
	@echo "\e[1;32m\n_____BUILDING_EXECUTABLES_____\e[0m"
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)
	$(MAKE) -C $(SOURCE_DIR) all

update-img: $(IMAGE) build
	@echo "\e[1;32m\n_____BUILDING_IMAGE_____\e[0m"
	mformat -i $(IMAGE) -F ::
	mmd -i $(IMAGE) ::/EFI
	mmd -i $(IMAGE) ::/EFI/BOOT
	mmd -i $(IMAGE) ::/BOOT
	mcopy -i $(IMAGE) $(BUILD_DIR)/bootx64/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $(IMAGE) $(BUILD_DIR)/bootloader/MDOSBOOT.EFI ::/BOOT
	mcopy -si $(IMAGE) $(FILES_DIR)/* ::

gen-keys: $(KEY_HDR)

run:
	$(EMU) $(EMU_BASE_FLAGS)

run-extra-info:
	$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS)

debug:
	$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS) &
	$(DBG) $(DBG_FLAGS)

clean-all: clean
	$(MAKE) -C gnu-efi clean
	rm -rf $(GNU_EFI_BUILT_NOTE)
	rm -rf $(BUILD_DIR)

clean:
	find $(SOURCE_DIR) -name "*.o" -type f -delete
	find $(SOURCE_DIR) -name "*.so" -type f -delete
	find $(BUILD_DIR) -name "*.o" -type f -delete
	find $(BUILD_DIR) -name "*.a" -type f -delete
	find $(BUILD_DIR) -name "*.so" -type f -delete
	find $(BUILD_DIR) -name "*.efi" -type f -delete
	find $(BUILD_DIR) -name "*.efi.debug" -type f -delete
	find $(BUILD_DIR) -name "*.elf" -type f -delete

$(GNU_EFI_BUILT_NOTE):
	$(MAKE) -C $(GNU_EFI_DIR) all
	@touch $@

$(IMAGE):
	@mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$(IMAGE) bs=512 count=93750

$(PUB_KEY) $(SEC_KEY):
	@echo "Generating keypair"
	mkdir -p $(KEYS_DIR)
	md-keygen -pk $(PUB_KEY) -sk $(SEC_KEY)

$(KEY_HDR): $(PUB_KEY)
	@echo "#ifndef PUBLIC_KEY_H" 	> $@
	@echo "#define PUBLIC_KEY_H" 	>> $@
	@echo "#ifdef __cplusplus" 		>> $@
	@echo 'extern "C" {'			>> $@
	@echo "#endif" 					>> $@
	@echo ""						>> $@
	@echo "#include <stdint.h>"		>> $@	
	@echo ""						>> $@
	md-keytoarr -in $< -out $@ --var public_key
	@echo ""						>> $@
	@echo "#ifdef __cplusplus" 		>> $@
	@echo '}'						>> $@
	@echo "#endif" 					>> $@
	@echo "#endif" 					>> $@

$(KEY_HDR): $(PUB_KEY)