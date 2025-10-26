OBJTYPE := dbg
$(OBJTYPE)-SRC := $(patsubst $(CURDIR)/%,%,$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

$(OBJTYPE)-objs := dbgio.o

$(OBJTYPE)-objs := $(addprefix $($(OBJTYPE)-SRC)/,$($(OBJTYPE)-objs))
obj-y += $($(OBJTYPE)-objs)