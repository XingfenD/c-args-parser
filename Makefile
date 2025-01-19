# ./Makefile

CC = gcc
CFLAGS = -Wall -g $(INCLUDES) -Wextra -funroll-loops -march=native
LDFLAGS =
INCLUDES = -I./inc

LIB_DIR = lib
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)

SOURCES := $(wildcard $(LIB_DIR)/*.c)
OBJECTS := $(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
TEST_EXEC = $(BIN_DIR)/test_parser.out

all: $(TEST_EXEC)

clean:
	rm -rf build

.PHONY: clean

$(TEST_EXEC): $(OBJECTS) $(BUILD_DIR)/main.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/main.o: ./main.c $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir $@
