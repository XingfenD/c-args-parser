# ./Makefile
CC_c = gcc
CC_cpp = g++
CFLAGS = -Wall -g $(INCLUDES) -Wextra -funroll-loops -march=native
LDFLAGS =
INCLUDES = -I./inc

LIB_DIR = lib
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)

C_EXEC = $(BUILD_DIR)/test_c

# build targets
all: test_c

c_gen: CC = $(CC_c)
c_gen: $(C_EXEC)

test_c: CC = $(CC_c)
test_c: $(C_EXEC)
	$(C_EXEC) help

clean:
	rm -rf build

.PHONY: clean

# link targets

$(C_EXEC): $(BUILD_DIR)/scap.o $(BUILD_DIR)/test_c.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# compile targets

$(BUILD_DIR)/test_c.o:./test_c.c $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/scap.o: $(LIB_DIR)/scap.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir $@
