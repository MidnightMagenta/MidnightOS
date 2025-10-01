SRC := $(patsubst $(CURDIR)/%,%,$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

boot-objs := entry.o start.o
boot-objs := $(addprefix $(SRC)/,$(boot-objs))
obj-y += $(boot-objs)