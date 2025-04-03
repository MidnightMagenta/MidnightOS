CC = .
AC = .
LD = .

LDS = kernel_linker.ld

BUILD_DIR = .
SOURCE_DIR = .
LIBC_DIR = $(SOURCE_DIR)/../libc

ELF_TARGET = kernel.elf

C_SRCS = $(shell find $(SOURCE_DIR) -name '*.c') $(shell find $(LIBC_DIR) -name '*.c')
C_HDRS = $(shell find $(SOURCE_DIR) -name '*.h') $(shell find $(LIBC_DIR) -name '*.h')
C_OBJS = $(patsubst $(SOURCE_DIR)%.cpp, $(BUILD_DIR)%.o, $(C_SRCS))

CPP_SRCS = $(shell find $(SOURCE_DIR) -name '*.cpp') $(shell find $(LIBC_DIR) -name '*.cpp')
CPP_HDRS = $(shell find $(SOURCE_DIR) -name '*.hpp') $(shell find $(LIBC_DIR) -name '*.hpp')
CPP_OBJS = $(patsubst $(SOURCE_DIR)%.cpp, $(BUILD_DIR)%.o, $(CPP_SRCS))

ASM_SRCS = $(shell find $(SOURCE_DIR) -name '*.asm')
ASM_OBJS = $(patsubst $(SOURCE_DIR)%.asm, $(BUILD_DIR)%_asm.o, $(ASM_SRCS))

OBJS = $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)

# Default flags
CFLAGS = -g -ffreestanding -fshort-wchar -mno-red-zone -m64 -Wall -nostdlib -nostdinc \
		-fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
ACFLAGS = -f elf64
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib

all: $(OBJS)
	$(LD) -T $(LDS) $(LDFLAGS) -o $(BUILD_DIR)/$(ELF_TARGET) $(OBJS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(C_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(abspath $(LIBC_DIR)) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(CPP_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(abspath $(LIBC_DIR)) -c $< -o $@

$(BUILD_DIR)/%_asm.o: $(SOURCE_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AC) $(ACFLAGS) -o $@ $<