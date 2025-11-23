CC ?= gcc
CXX ?= g++
FLEX ?= flex
BISON ?= bison

CFLAGS := -O0 -g -MP -MMD
CXXFLAGS := $(CFLAGS)
CPPFLAGS :=
LDFLAGS := -lfl

TARGET := mbuild

obj-p := parser.o lexer.o
obj := $(obj-p) ast.o main.o eval.o emit.o

.PHONY: all clean ccdb

all: $(TARGET)

$(TARGET): $(obj)
	@echo -e "LD $@"
	@$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	@find ./ -name "*.o" -type f -delete
	@rm -f compile_commands.json
	@rm -rf .cache
	@rm -f $(TARGET)
	@rm -f $(obj:.o=.d)
	@rm -f $(obj-p:.o=.c)
	@rm -f $(obj-p:.o=.h)

%.o: %.c
	@echo -e "CC $@"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.o: %.cc
	@echo -e "CXX $@"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

%.c: %.l
	@echo -e "FLEX $@"
	@$(FLEX) -o $@ $<

%.c: %.y
	@echo -e "BISON $@"
	@$(BISON) -d -o $@ $<

-include $(obj:.o=.d)

ccdb:
	@compiledb make -Bn all