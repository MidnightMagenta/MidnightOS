CC = .
AC = .
LD = .

LDS = kernel_linker.ld

BUILD_DIR = .
SOURCE_DIR = .
FILES_DIR = .
LIBC_DIR = $(SOURCE_DIR)/../libc
IMAGE_DIR = $(FILES_DIR)/KRNL

CFLAGS = -g -ffreestanding -fshort-wchar -mno-red-zone -m64 -Wall -nostdlib -nostdinc \
		-fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
ACFLAGS = -f elf64
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib

all: build_kernel 

build_kernel: 
	$(MAKE) -C $(SOURCE_DIR)/kernel BUILD_DIR="$(BUILD_DIR)/kernel" \
									FILES_DIR="$(FILES_DIR)" \
									CC="$(CC)" \
									LD="$(LD)" \
									AC="$(AC)" \
									CFLAGS="$(CFLAGS)" \
									LDFLAGS="$(LDFLAGS)" \
									ACFLAGS="$(ACFLAGS)" \
									all