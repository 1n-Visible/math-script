#include "charset.h"

/*
alphanum[] = (L"_"
    L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    L"abcdefghijklmnopqrstuvwxyz"
    L"ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ"
    L"αβγδεζηθικλμνξοπρστυφχψωςϕ"
);
symbols[] = L" !\"#$%&'()*+,-./:;<=>?@[\\]^{|}~±²³Ø∂∈∉√∞∫≈≠≤≥⊂⊃";
whitespace[] = L" \n\t";
single_chars[] = L"#$(),.:;?[]{}±²³Ø∂∈∉√∞∫≈≠≤≥⊂⊃",
*/

extern bool valid_whitespace(wchar_t c) {
    return (c==L' ') or (c==L'\t') or (c==L'\n');
}

extern bool valid_alpha(wchar_t c) {
    return (
        (L'A'<=c and c<=L'Z') or (L'a'<=c and c<=L'z') or
        (L'Α'<=c and c<=L'ω' and (c<=L'Ω' or L'α'<=c) and c!=L'\x3a2') or
        (c==L'ϕ') or (c==L'_')
    );
}

extern bool valid_digit(wchar_t c) {
    return L'0'<=c and c<=L'9';
}

extern bool valid_hexdigit(wchar_t c) {
    return (L'0'<=c and c<=L'9') or (L'a'<=c and c<=L'f');
}

extern bool valid_alnum(wchar_t c) {
    return valid_alpha(c) or valid_digit(c);
}

extern bool valid_varname(wchar_t *name) {
    wchar_t c=*name;
    if (!valid_alpha(c))
        return false;

    size_t i;
    for (i=1; (c=name[i])!=L'\0'; i++) {
        if (!valid_alnum(c))
            return false;
    }

    return true;
}

short alnum_to_index(wchar_t c) { //TODO: case L'A' ... L'Z':
    if (c<=L'9')
        return c-L'0';
    if (c<=L'Z')
        return c-L'A'+10;
    if (c==L'_')
        return 36;
    if (c<=L'z')
        return c-L'a'+37;
    if (c<=L'Ρ')
        return c-L'Α'+63;
    if (c<=L'Ω')
        return c-L'Σ'+80;
    if (c<=L'ω')
        return c-L'α'+87;
    if (c==L'ϕ')
        return 112;
    return -1;
}

wchar_t index_to_alnum(short index) {
    if (index<10)
        return index+L'0';
    if (index<36)
        return index+L'A'-10;
    if (index==36)
        return L'_';
    if (index<63)
        return index+L'a'-37;
    if (index<80)
        return index+L'Α'-63;
    if (index<87)
        return index+L'Σ'-80;
    if (index<112)
        return index+L'α'-87;
    if (index==112)
        return L'ϕ';
    return L'\0';
}

wchar_t *string_copy(const wchar_t *string) {
    if (string==NULL)
        return NULL;
    
    wchar_t *new_string=calloc(wcslen(string)+1, sizeof(wchar_t));
    wcscpy(new_string, string);
    return new_string;
}

wchar_t *string_copy_len(const wchar_t *string, size_t *length) {
    if (string==NULL) {
        *length=0;
        return NULL;
    }
    
    *length=wcslen(string);
    wchar_t *new_string=calloc(*length+1, sizeof(wchar_t));
    wcscpy(new_string, string);
    return new_string;
}
