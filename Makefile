CC=gcc

WARNINGS=-Wall -Wpedantic -Wno-error \
-Wstrict-aliasing -Wfree-nonheap-object \
-Wcast-align=strict -Wcast-function-type -Wno-int-to-pointer-cast \
-Wlarger-than=4096 -Wframe-larger-than=256 \
-Woverflow -Wpacked -Wreturn-local-addr -Wsuggest-attribute=const \
-Wno-switch -Wno-unused-variable -Wno-discarded-qualifiers
DEBUG=
OPTIONS=-O0 -fshort-enums -finline-small-functions
LINKS=-lm #link math.h
CFLAGS=$(DEBUG) $(WARNINGS) $(OPTIONS) $(LINKS)

BIN=bin/math-script
BUILD=build
SRC=src
#SRCS=$(wildcard $(SRC)/*.c)
SRCS=$(find . $(SRC)/*.c)
OBJECTS=$(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SRCS))

all: $(BIN)

$(BIN): $(OBJECTS) rt_values.o
	$(CC) $(CFLAGS) $^ -o $@

rt_values.o:
	$(CC) $(CFLAGS) -c src/rt_values/*.c -o build/rt_values.o

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r bin/* build/*
