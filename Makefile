# ./Makefile
CC_c = gcc
CC_cpp = g++
CFLAGS = -Wall -g $(INCLUDES) -Wextra -funroll-loops -march=native
LDFLAGS =
INCLUDES = -I./inc

LIB_DIR = lib
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)

CPP_EXEC = $(BUILD_DIR)/test_cpp
C_EXEC = $(BUILD_DIR)/test_c

# build targets

c_gen: CC = $(CC_c)
c_gen: $(C_EXEC)

cpp_gen: CC = $(CC_cpp)
cpp_gen: $(CPP_EXEC)

test_c: CC = $(CC_c)
test_c: $(C_EXEC)
	$(C_EXEC) help

test_cpp: CC = $(CC_cpp)
test_cpp: $(CPP_EXEC)
	$(CPP_EXEC) help

clean:
	rm -rf build

.PHONY: clean

# link targets

$(CPP_EXEC): $(BUILD_DIR)/scppap.o $(BUILD_DIR)/test_cpp.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(C_EXEC): $(BUILD_DIR)/scap.o $(BUILD_DIR)/test_c.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# compile targets

$(BUILD_DIR)/test_c.o:./test_c.c $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/test_cpp.o: ./test_cpp.cpp $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/scap.o: $(LIB_DIR)/scap.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/scppap.o: $(LIB_DIR)/scppap.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir $@
