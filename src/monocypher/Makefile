include $(MAKE_VARS)

LIB_TARGET = libmonocypher.a
OBJS = monocypher.o

.PHONY: all link
.NOTPARALLEL: all

all: link

link: $(OBJS)
	@echo "\e[1;32m\n_____ARCHIVING_____\e[0m"
	ar rcs $(LIB_DIR)/$(LIB_TARGET) $(OBJS)

monocypher.o: monocypher.c monocypher.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@