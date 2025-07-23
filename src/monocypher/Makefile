include $(MAKE_VARS)

LIB_TARGET = libmonocypher.a
OBJS = monocypher.o

.PHONY: all

all: $(LIB_DIR)/$(LIB_TARGET)

$(LIB_DIR)/$(LIB_TARGET): $(OBJS)
	@echo "\e[1;32m\n_____ARCHIVING_____\e[0m"
	ar rcs $(LIB_DIR)/$(LIB_TARGET) $(OBJS)

monocypher.o: monocypher.c monocypher.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@