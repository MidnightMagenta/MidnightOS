EMU := 
DBG := 

ifeq ($(ARCH),x86_64)
  EMU := qemu-system-x86_64
  DBG := gdb
else
  $(error Unsuported architecture $(ARCH))
  # TODO: implement other arches
endif

OVMF_BINS := ovmf-bins/$(ARCH)

EMU_BASE_FLAGS = -drive file=$(IMAGE),format=raw \
				-m 2G \
				-cpu qemu64 \
				-vga std \
				-drive if=pflash,format=raw,unit=0,file="$(OVMF_BINS)/OVMF_CODE-pure-efi.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="$(OVMF_BINS)/OVMF_VARS-pure-efi.fd" \
				-net none \
				-machine q35 \
				-nographic

EMU_DBG_FLAGS = -s -S -d int,guest_errors,cpu_reset -no-reboot -no-shutdown -D tmp/qemu.log

DBG_FLAGS = -ex "symbol-file $(BUILD_DIR)/$(KERNEL_TARGET)" \
			-ex "target remote localhost:1234" \
			-ex "set disassemble-next-line on" \
			-ex "set step-mode on"

.PHONY += run run-info run-debug

run:
	$(EMU) $(EMU_BASE_FLAGS)

run-info:
	@mkdir -p ./tmp
	$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS)

run-debug:
	@mkdir -p ./tmp
	@$(EMU) $(EMU_BASE_FLAGS) $(EMU_DBG_FLAGS) &
	@$(DBG) $(DBG_FLAGS)
