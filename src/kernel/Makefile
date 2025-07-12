CC = .
AC = .
LD = .

LDS = kernel_linker.ld

BUILD_DIR = .
SOURCE_DIR = .
FILES_DIR = .
IMAGE_DIR = $(FILES_DIR)/KRNL

ELF_TARGET = kernel.elf

CRTI_SRC = $(shell find $(SOURCE_DIR) -name 'crti.S')
CRTN_SRC = $(shell find $(SOURCE_DIR) -name 'crtn.S')

CRTI_OBJ =  $(patsubst $(SOURCE_DIR)%.S, $(BUILD_DIR)%.o, $(CRTI_SRC))
CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ =  $(patsubst $(SOURCE_DIR)%.S, $(BUILD_DIR)%.o, $(CRTN_SRC))

C_SRCS = $(shell find $(SOURCE_DIR) -name '*.c')
C_HDRS = $(shell find $(SOURCE_DIR) -name '*.h')
C_OBJS = $(patsubst $(SOURCE_DIR)%.c, $(BUILD_DIR)%.o, $(C_SRCS))

CPP_SRCS = $(shell find $(SOURCE_DIR) -name '*.cpp')
CPP_HDRS = $(shell find $(SOURCE_DIR) -name '*.hpp')
CPP_OBJS = $(patsubst $(SOURCE_DIR)%.cpp, $(BUILD_DIR)%.o, $(CPP_SRCS))

ASM_SRCS = $(shell find $(SOURCE_DIR) -name '*.asm')
ASM_OBJS = $(patsubst $(SOURCE_DIR)%.asm, $(BUILD_DIR)%_asm.o, $(ASM_SRCS))

OBJS = $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)

INCLUDE_DIRS = \
	-Iklibc/include \
	-IIO/include \
	-Imemory/include \
	-Iboot/include \
	-Ik_utils/include \
	-Ierror/include \
	-Imain/include

# Default flags
CFLAGS = -g -ffreestanding -fshort-wchar -mno-red-zone -m64 -Wall -nostdlib -nostdinc \
		-fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
CPPFLAGS = $(CFLAGS)
ACFLAGS = -f elf64
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib

all: $(OBJS) link copy_build_files

link:
	@echo "\e[1;32m\n_____LINKING_____\e[0m"
	$(LD) -T $(LDS) $(LDFLAGS) -o $(BUILD_DIR)/$(ELF_TARGET) $(OBJS)

copy_build_files:
	@echo "\e[1;32m\n_____COPYING_FILES_____\e[0m"
	mkdir -p $(IMAGE_DIR)
	cp $(BUILD_DIR)/$(ELF_TARGET) $(IMAGE_DIR)/$(ELF_TARGET)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(C_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(CPP_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_asm.o: $(SOURCE_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AC) $(ACFLAGS) -o $@ $<