include $(MAKE_VARS)
include $(MAKE_TARGETS)

LOCAL_SOURCE_DIR := .

LDS = kernel_linker.ld

BUILD_TARGET = $(KERNEL_BUILD_TARGET)

C_SRCS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.c')
C_HDRS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.h')
C_OBJS = $(patsubst $(LOCAL_SOURCE_DIR)%.c, $(KERNEL_BUILD_DIR)%.o, $(C_SRCS))

ASM_SRCS = $(shell find $(LOCAL_SOURCE_DIR) -name '*.asm')
ASM_OBJS = $(patsubst $(LOCAL_SOURCE_DIR)%.asm, $(KERNEL_BUILD_DIR)%_asm.o, $(ASM_SRCS))

OBJS = $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)

INCLUDE_DIRS = 

LINKED_LIBS = 

.PHONY: all copy_build_files

all: $(BUILD_TARGET)

$(BUILD_TARGET): $(OBJS)
	@echo "\e[1;32mLinking:\e[0m $@"
	@$(LD) -T $(LDS) $(LDFLAGS) $(OBJS) $(LINKED_LIBS) -o $(BUILD_TARGET)
	
$(KERNEL_BUILD_DIR)/%.o: $(LOCAL_SOURCE_DIR)/%.c $(C_HDRS)
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(KERNEL_BUILD_DIR)/%.o: $(LOCAL_SOURCE_DIR)/%.S
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BUILD_DIR)/%_asm.o: $(LOCAL_SOURCE_DIR)/%.asm
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	@$(AC) $(ACFLAGS) -o $@ $<