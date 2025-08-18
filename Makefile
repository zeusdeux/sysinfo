CC ?= clang

MACOS_VERSION = $(shell sw_vers -productVersion | cut -c1-4)
X86_64_TRIPLE = x86_64-apple-macos$(MACOS_VERSION)

INCLUDES=-I./
CFLAGS=-std=c17 -Wall -Wdeprecated -Werror -Wextra -Wpedantic -pedantic \
	$(INCLUDES)

RELEASE_FLAGS=-DNDEBUG -O3
DEBUG_FLAGS=-g -O0

SOURCES=main.c
BUILD_DIR=./.build


all: clean debug release

ensure:
	mkdir -p $(BUILD_DIR)

debug: ensure $(SOURCES)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SOURCES) -o $(BUILD_DIR)/sysinfo-arm64-dbg
	$(CC) -target $(X86_64_TRIPLE) $(CFLAGS) $(DEBUG_FLAGS) $(SOURCES) -o $(BUILD_DIR)/sysinfo-x86_64-dbg
	lipo -create -output $(BUILD_DIR)/sysinfo-dbg $(BUILD_DIR)/sysinfo-x86_64-dbg $(BUILD_DIR)/sysinfo-arm64-dbg

release: ensure $(SOURCES)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(SOURCES) -o $(BUILD_DIR)/sysinfo-arm64
	$(CC) -target $(X86_64_TRIPLE) $(CFLAGS) $(RELEASE_FLAGS) $(SOURCES) -o $(BUILD_DIR)/sysinfo-x86_64
	lipo -create -output $(BUILD_DIR)/sysinfo $(BUILD_DIR)/sysinfo-x86_64 $(BUILD_DIR)/sysinfo-arm64

clean:
	rm -fr $(BUILD_DIR)

dump: debug
	objdump -Sds $(BUILD_DIR)/main-dbg

.PHONY: all ensure debug release clean dump
