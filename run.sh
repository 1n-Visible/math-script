#!/bin/bash

# GCC version >= 4.4

set -e #-Wpedantic
echo -e '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n'
WARNINGS="-Wall -Wno-error -Wstrict-aliasing \
-Wfree-nonheap-object \
-Wcast-align=strict -Wcast-function-type -Wno-int-to-pointer-cast \
-Wlarger-than=4096 -Wframe-larger-than=256 \
-Woverflow -Wpacked -Wreturn-local-addr -Wsuggest-attribute=const \
-Wno-switch -Wno-unused-variable -Wno-discarded-qualifiers"

OPTIONS="-g -fshort-enums -finline-small-functions"
#-fcx-fortran-rules? -ffast-math -fpack-struct

LINKS="-lm"
FILES="src/*.c src/rt_values/*.c"
#"main.c lexer.c parser.c interpreter.c \
#fraction.c intmath.c dict_tree.c charset.c"

gcc $WARNINGS $OPTIONS $LINKS $FILES -o math-script

./math-script -s
