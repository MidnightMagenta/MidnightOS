# Includes a diroectory and defines a subdir variable with the path to that directory
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

KERNEL_OBJS := $(patsubst %.o,$(BUILD_DIR)/%.o,$(obj-y))
DEP_OBJS := $(KERNEL_OBJS:.o=.d)

$(BUILD_DIR)/$(KERNEL_TARGET): $(KERNEL_OBJS)
	@echo -e "\e[1;32mLinking:\e[0m $@"
	@$(LD) $(LDFLAGS) -T link/$(ARCH)-link.ld -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@echo -e "Compiling: $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.asm
	@echo -e "Assembling: $<"
	@mkdir -p $(@D)
	$(AC) $(ACFLAGS) -o $@ $<

-include $(DEP_OBJS)
