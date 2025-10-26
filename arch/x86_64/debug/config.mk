OBJTYPE := arch-debug
$(OBJTYPE)-SRC := $(patsubst $(CURDIR)/%,%,$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

$(OBJTYPE)-objs := dbg_serial.o

$(OBJTYPE)-objs := $(addprefix $($(OBJTYPE)-SRC)/,$($(OBJTYPE)-objs))
obj-y += $($(OBJTYPE)-objs)