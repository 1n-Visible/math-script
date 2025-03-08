#ifndef CHARSET_H
#define CHARSET_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#define CHARSET_SIZE 113

extern const wchar_t *valid_symbols;
bool valid_whitespace(wchar_t);
bool valid_alpha(wchar_t);
bool valid_alnum(wchar_t);
bool valid_varname(wchar_t *);

short hexdigit_to_num(wchar_t);
short alnum_to_index(wchar_t);
wchar_t index_to_alnum(short index);

wchar_t *wcsdup_len(const wchar_t *wstring, size_t *len_ptr);
wchar_t *str_to_wcs(const char *string, size_t length);

#endif
