#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>
#include <iso646.h>

#ifndef CHARSET_H
#define CHARSET_H

#define CHARSET_SIZE 113

bool valid_whitespace(wchar_t c);
bool valid_alpha(wchar_t c);
bool valid_digit(wchar_t c);
bool valid_hexdigit(wchar_t c);
bool valid_alnum(wchar_t c);
bool valid_varname(wchar_t *name);

short alnum_to_index(wchar_t c);
wchar_t index_to_alnum(short index);

wchar_t *string_copy(const wchar_t *string);
wchar_t *string_copy_len(const wchar_t *string, size_t *length);

#endif
