include $(MAKE_VARS)

LOCAL_SOURCE_DIR := .

LDS = kernel_linker.ld
IMAGE_DIR = $(FILES_DIR)/KRNL

ELF_TARGET = kernel.elf

CRTI_SRC = $(shell find $(LOCAL_SOURCE_DIR) -name 'crti.S')
CRTN_SRC = $(shell find $(LOCAL_SOURCE_DIR) -name 'crtn.S')

CRTI_OBJ =  $(patsubst $(LOCAL_SOURCE_DIR)%.S, $(BUILD_DIR)%.o, $(CRTI_SRC))
CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ =  $(patsubst $(LOCAL_SOURCE_DIR)%.S, $(BUILD_DIR)%.o, $(CRTN_SRC))

C_SRCS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.c')
C_HDRS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.h')
C_OBJS = $(patsubst $(LOCAL_SOURCE_DIR)%.c, $(BUILD_DIR)%.o, $(C_SRCS))

CPP_SRCS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.cpp')
CPP_HDRS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.hpp')
CPP_OBJS = $(patsubst $(LOCAL_SOURCE_DIR)%.cpp, $(BUILD_DIR)%.o, $(CPP_SRCS))

ASM_SRCS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.asm')
ASM_OBJS = $(patsubst $(LOCAL_SOURCE_DIR)%.asm, $(BUILD_DIR)%_asm.o, $(ASM_SRCS))

OBJS = $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)

INCLUDE_DIRS = \
	-I../monocypher \
	-Ilibk/include \
	-IIO/include \
	-Imemory/include \
	-Iboot/include \
	-Ik_utils/include \
	-Ierror/include \
	-Imain/include \
	-Ithread/include

LINKED_LIBS = \
	-lmonocypher

.PHONY: all link copy_build_files

all: $(OBJS) link copy_build_files

link:
	@echo "\e[1;32m\n_____LINKING_____\e[0m"
	$(LD) -T $(LDS) $(LDFLAGS) $(OBJS) $(LINKED_LIBS) -o $(BUILD_DIR)/$(ELF_TARGET)

copy_build_files:
	@echo "\e[1;32m\n_____COPYING_FILES_____\e[0m"
	mkdir -p $(IMAGE_DIR)
	cp $(BUILD_DIR)/$(ELF_TARGET) $(IMAGE_DIR)/$(ELF_TARGET)

$(BUILD_DIR)/%.o: $(LOCAL_SOURCE_DIR)/%.c $(C_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LOCAL_SOURCE_DIR)/%.cpp $(CPP_HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LOCAL_SOURCE_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_asm.o: $(LOCAL_SOURCE_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AC) $(ACFLAGS) -o $@ $<