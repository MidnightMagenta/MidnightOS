CC = .

BUILD_DIR = .

LIB_TARGET = libmonocypher.a

OBJS = monocypher.o

# Default flags
CFLAGS ?= -g -ffreestanding -fshort-wchar -mno-red-zone -m64 -Wall -nostdlib -nostdinc \
		-fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

all: $(OBJS) link

link:
	@echo "\e[1;32m\n_____ARCHIVING_____\e[0m"
	ar rcs $(LIB_DIR)/$(LIB_TARGET) $(OBJS)

monocypher.o: monocypher.c monocypher.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@