BUILD_DIR := .
INCLUDE_DIR := .

.PHONY: all

all:
	@$(MAKE) -C bootx64 BUILD_DIR="$(BUILD_DIR)" all
	@$(MAKE) -C mdosboot BUILD_DIR="$(BUILD_DIR)" INCLUDE_DIR="$(INCLUDE_DIR)" all
