# Includes a directory and defines a subdir variable with the path to that directory
# relative to the root directory of the project
define include-dir
  subdir := $(1)
  include $(1)/Makefile
  included-dirs += $(1)
endef

# include recursively all subdirectories in a list of objects
define include-tree
  $(foreach d,$(filter %/,$(1)), \
    $(if $(filter $(d),$(visited-dirs)),, \
      $(eval visited-dirs += $(d)) \
      $(eval $(call include-dir,$(d))) \
      $(call include-tree,$(filter %/,$(obj-y))) \
    ) \
  )
endef

$(eval $(call include-tree,$(filter %/,$(obj-y))))
obj-y := $(filter-out %/,$(obj-y))

$(eval $(call include-tree,$(filter %/,$(lib-y))))
lib-y := $(filter-out %/,$(lib-y))

KERNEL_OBJS := $(patsubst %.o,$(BUILD_DIR)/%.o,$(obj-y))
KLIB_OBJS := $(patsubst %.o,$(BUILD_DIR)/%.o,$(lib-y))

$(BUILD_DIR)/$(KERNEL_TARGET): $(KERNEL_OBJS) $(LIB_DIR)/$(KLIB_TARGET)
	@echo -e "\e[1;32mLinking:\e[0m $@"
	@mkdir -p $(@D)
	@$(LD) $(LDFLAGS) -T link/$(ARCH)-link.ld -lnyx -o $@ $^

$(LIB_DIR)/$(KLIB_TARGET): $(KLIB_OBJS)
	@echo -e "\e[1;32mLinking:\e[0m $@"
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: %.c
	@echo -e "Compiling: $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $($*-cflags) -c -o $@ $<

$(BUILD_DIR)/%.o: %.asm
	@echo -e "Assembling: $<"
	@mkdir -p $(@D)
	@$(AC) $(ACFLAGS) -i$(dir $<) -MD "$(@:.o=.d)" -o $@ $<

$(BUILD_DIR)/%.o: %.S
	@echo -e "Assembling: $<"
	@mkdir -p $(@D)
	@$(CC) -x assembler-with-cpp -D_ASSEMBLY_ $(CFLAGS) $($*-acflags) -c -o $@ $<
	@rm -f $(<:%.S=%.s)

DEP_OBJS := $(KERNEL_OBJS:.o=.d) $(KLIB_OBJS:.o=.d)

-include $(DEP_OBJS)
