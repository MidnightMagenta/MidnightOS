SRC := $(patsubst $(CURDIR)/%,%,$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

-include $(SRC)/boot/config.mk