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
OVMF_BINARIES_DIR := $(abspath $(TOPLEVEL_DIR)ovmf-bins)
GNU_EFI_DIR := $(abspath $(TOPLEVEL_DIR)gnu-efi)

UPDATEIMG_SH := $(SHELL_DIR)/updateimg.sh
IMAGE = $(BUILD_DIR)/disk.img

C_COMPILE_DEFS = -D_DEBUG
C_OP_LVL = -O0 -fno-tree-vectorize
C_F_FLAGS = -ffreestanding -fshort-wchar -fno-omit-frame-pointer -fno-builtin -fno-stack-protector 						\
			-fno-exceptions -fno-builtin-memcpy -fno-builtin-memset
C_W_FLALGS = -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wundef 											\
			 -Wcast-align -Wshift-overflow -Wdouble-promotion -Werror
CFLAGS = -g -mno-red-zone -m64 -mcmodel=kernel -nostartfiles -nodefaultlibs -nostdlib $(C_F_FLAGS) $(C_COMPILE_DEFS)
ACFLAGS = -f elf64
LDFLAGS = -static -Bsymbolic -nostdlib -L$(LIB_DIR)