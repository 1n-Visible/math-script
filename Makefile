.PHONY: all clean
.SILENT:

CC=gcc

WARNINGS=-Wall -Wpedantic -Wno-error \
-Wstrict-aliasing -Wfree-nonheap-object \
-Wcast-align=strict -Wcast-function-type -Wno-int-to-pointer-cast \
-Wlarger-than=4096 -Wframe-larger-than=256 \
-Woverflow -Wpacked -Wreturn-local-addr -Wsuggest-attribute=const \
-Wno-switch -Wno-unused-variable -Wno-discarded-qualifiers
DEBUG=--debug #-O2
OPTIONS=-fshort-enums -finline-small-functions #-fmax-include-depth=10
LINKS=-Iinclude -lm #link math.h
CFLAGS=$(DEBUG) $(WARNINGS) $(OPTIONS) $(LINKS)

BIN=bin/math-script
#SRCS=$(wildcard $(SRC)/*.c)
SRCS=$(find . src/*.c)
OBJECTS=$(patsubst src/%.c, build/%.o, $(SRCS))

SRC_SUBDIRS=$(shell find src -type d)
BUILD_SUBDIRS=$(patsubst src%,build%,$(SRC_SUBDIRS))

SRC_FILES=$(foreach dir,$(SRC_SUBDIRS),$(wildcard $(dir)/*.c))
BUILD_FILES=$(patsubst src/%.c,build/%.o,$(SRC_FILES))

all: $(BUILD_SUBDIRS) $(BIN)

$(BIN): $(BUILD_FILES)
	mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_SUBDIRS):
	mkdir -p $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf bin/* build/*
