include $(MAKE_VARS)

LIB_TARGET = libmonocypher.a
OBJS = monocypher.o

all: $(OBJS) link

link:
	@echo "\e[1;32m\n_____ARCHIVING_____\e[0m"
	ar rcs $(LIB_DIR)/$(LIB_TARGET) $(OBJS)

monocypher.o: monocypher.c monocypher.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@