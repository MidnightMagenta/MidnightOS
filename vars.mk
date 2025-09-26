EMU := qemu-system-x86_64
DBG := gdb
PREFIX := x86_64-elf-

CC := $(PREFIX)gcc
AC := nasm
LD := $(PREFIX)ld
OBJCPY := $(PREFIX)objcopy
SIGN := md-sign --objcpy $(OBJCPY)

BUILD_DIR := $(abspath $(TOPLEVEL_DIR)/build)
LIB_DIR := $(BUILD_DIR)/lib
SOURCE_DIR := $(abspath $(TOPLEVEL_DIR)/src)
FILES_DIR := $(abspath $(TOPLEVEL_DIR)/files)
SHELL_DIR := $(abspath $(TOPLEVEL_DIR)/shell)
OVMF_BINARIES_DIR := $(abspath $(TOPLEVEL_DIR)/ovmf-bins)
GNU_EFI_DIR := $(abspath $(TOPLEVEL_DIR)/gnu-efi)
GNU_EFI_BUILT_NOTE := $(BUILD_DIR)/.gnu-efi-note

UPDATEIMG_SH := $(SHELL_DIR)/updateimg.sh
IMAGE = $(BUILD_DIR)/disk.img

# build flags
C_COMPILE_DEFS = -D_DEBUG
C_OP_LVL = -O0 -fno-tree-vectorize
C_F_FLAGS = -ffreestanding -fshort-wchar -fno-omit-frame-pointer -fno-builtin -fno-stack-protector 						\
			-fno-exceptions -fno-builtin-memcpy -fno-builtin-memset
C_W_FLALGS = -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wundef 											\
			 -Wcast-align -Wshift-overflow -Wdouble-promotion -Werror
CFLAGS = -g -mno-red-zone -m64 -mcmodel=kernel -nostartfiles -nodefaultlibs -nostdlib $(C_F_FLAGS) $(C_COMPILE_DEFS)
ACFLAGS = -f elf64
LDFLAGS = -static -Bsymbolic -nostdlib -L$(LIB_DIR)

# debuging flags
EMU_BASE_FLAGS = -drive file=$(IMAGE),format=raw 																		\
				-m 2G 																									\
				-cpu qemu64 																							\
				-vga std																								\
				-drive if=pflash,format=raw,unit=0,file="$(OVMF_BINARIES_DIR)/OVMF_CODE-pure-efi.fd",readonly=on 		\
				-drive if=pflash,format=raw,unit=1,file="$(OVMF_BINARIES_DIR)/OVMF_VARS-pure-efi.fd" 					\
				-net none 																								\
				-machine q35

EMU_DBG_FLAGS = -s -S -d guest_errors,cpu_reset,int -no-reboot -no-shutdown

DBG_FLAGS = -ex "symbol-file $(BUILD_DIR)/kernel/kernel.elf" 															\
			-ex "target remote localhost:1234" 																			\
			-ex "set disassemble-next-line on" 																			\
			-ex "set step-mode on"

DISK_GUID = f953b4de-e77f-4f0b-a14e-2b29080599cf
ESP_GUID = 0cc13370-53ec-4cdb-8c3d-4185950e2581