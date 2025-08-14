CC=clang

INCLUDES=-I./
CFLAGS=-std=c17 -Wall -Wdeprecated -Werror -Wextra -Wpedantic -pedantic \
	$(INCLUDES)

RELEASE_FLAGS=-DNDEBUG -O3
DEBUG_FLAGS=-g -O0

SOURCES=main.c
BUILD_DIR=./.build

ensure:
	mkdir -p $(BUILD_DIR)

debug: ensure $(SOURCES)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SOURCES) -o $(BUILD_DIR)/main-dbg

release: ensure $(SOURCES)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(SOURCES) -o $(BUILD_DIR)/main-release

clean:
	rm -fr $(BUILD_DIR)

dump: debug
	objdump -Sds $(BUILD_DIR)/main-dbg

.PHONY: ensure debug release clean dump
