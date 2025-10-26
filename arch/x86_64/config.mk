ARCH_SRC := $(patsubst $(CURDIR)/%,%,$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

include $(ARCH_SRC)/debug/config.mk
include $(ARCH_SRC)/entry/config.mk
