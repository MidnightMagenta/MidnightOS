EMU := qemu-system-x86_64
DBG := gdb
PREFIX := x86_64-elf

CC := $(PREFIX)-gcc
AC := nasm
LD := $(PREFIX)-ld
OBJCPY := $(PREFIX)-objcopy
SIGN := md-sign --objcopy $(OBJCPY)

BUILD_DIR := $(abspath $(TOPLEVEL_DIR)/build)
LIB_DIR := $(BUILD_DIR)/lib
SOURCE_DIR := $(abspath $(TOPLEVEL_DIR)/src)
FILES_DIR := $(abspath $(TOPLEVEL_DIR)/files)
KEYS_DIR := $(abspath $(TOPLEVEL_DIR)/keys)
PUB_KEY := $(KEYS_DIR)/dev_public.key
SEC_KEY := $(KEYS_DIR)/dev_secret.key
KEY_HDR := $(KEYS_DIR)/public_key.h

OVMF_BINARIES_DIR := ovmf-bins
GNU_EFI_DIR := gnu-efi

LOG_VERBOSITY := 2

C_COMPILE_DEFS = -D_LOG_VERBOSITY=$(LOG_VERBOSITY) -D_DEBUG
C_OP_LVL = -O0
C_F_FLAGS = -ffreestanding -fshort-wchar -fno-omit-frame-pointer -fno-builtin -fno-stack-protector \
			-fno-exceptions -fno-tree-vectorize -fno-builtin-memcpy -fno-builtin-memset
C_W_FLALGS = -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wundef \
			 -Wcast-align -Wshift-overflow -Wdouble-promotion -Werror

CFLAGS = -g -mno-red-zone -m64 -mcmodel=kernel -nostartfiles -nodefaultlibs -nostdlib $(C_F_FLAGS) $(C_COMPILE_DEFS)
CPPFLAGS = $(CFLAGS) -fno-rtti -fno-use-cxa-atexit -std=c++20

ACFLAGS = -f elf64

LDFLAGS = -static -Bsymbolic -nostdlib -L$(LIB_DIR)