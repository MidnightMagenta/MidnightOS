CC = .
AC = .
LD = .

LDS = kernel_linker.ld

BUILD_DIR = .
SOURCE_DIR = .
FILES_DIR = .
LIB_DIR = .
IMAGE_DIR = $(FILES_DIR)/KRNL

C_W_FLALGS = -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wundef \
			 -Wcast-align -Wshift-overflow -Wdouble-promotion -Werror

CFLAGS ?= -g -ffreestanding -fshort-wchar -mno-red-zone -m64 -Wall -nostdlib -nostdinc \
		-fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
CPPFLAGS ?= $(CFLAGS)
ACFLAGS ?= -f elf64
LDFLAGS ?= -T $(LDS) -static -Bsymbolic -nostdlib

all: build_monocypher build_kernel

build_monocypher:
	@echo "\e[1;32m\n_____BUILDING_MONOCYPHER_____\e[0m"
	$(MAKE) -C $(SOURCE_DIR)/monocypher BUILD_DIR="$(LIB_DIR)" \
										CC="$(CC)" \
										CFLAGS="$(CFLAGS)" \
										all

build_kernel: 
	@echo "\e[1;32m\n_____BUILDING_KERNEL_____\e[0m"
	$(MAKE) -C $(SOURCE_DIR)/kernel BUILD_DIR="$(BUILD_DIR)/kernel" \
									FILES_DIR="$(FILES_DIR)" \
									LIB_DIR="$(LIB_DIR)" \
									CC="$(CC)" \
									LD="$(LD)" \
									AC="$(AC)" \
									CFLAGS="$(CFLAGS) $(C_W_FLALGS)" \
									CPPFLAGS="$(CPPFLAGS)" \
									LDFLAGS="$(LDFLAGS)" \
									ACFLAGS="$(ACFLAGS)" \
									all