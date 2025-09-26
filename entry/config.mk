SRC := $(patsubst $(CURDIR)/%,%,$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

obj-y += $(SRC)/entry.o $(SRC)/start.o