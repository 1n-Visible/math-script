#include "utils/charset.h"

const wchar_t *valid_symbols = L" !\"#$%&'()*+,-./:;<=>?@[\\]^{|}~±²³Ø∂∈∉√∞∫≈≠≤≥⊂⊃";

bool valid_whitespace(wchar_t c) {
    return (c==L' ') || (c==L'\t') || (c==L'\n');
}

bool valid_alpha(wchar_t c) {
    return (
        (L'A'<=c && c<=L'Z') || (L'a'<=c && c<=L'z') ||
        (L'Α'<=c && c<=L'Ω' && c!=L'\x3a2') || (L'α'<=c && c<=L'ω') ||
        (c==L'ϕ') || (c==L'_')
    );
}

bool valid_alnum(wchar_t c) {
    return valid_alpha(c) || iswdigit(c);
}

bool valid_varname(wchar_t *varname) {
    wchar_t c=varname[0];
    if (!valid_alpha(c))
        return false;

    for (size_t i=1; (c=varname[i])!=L'\0'; i++) {
        if (!valid_alnum(c)) return false;
    }

    return true;
}


short hexdigit_to_num(wchar_t c) {
    if (!iswxdigit(c))
        return -1;

    if (c<=L'9')
        return c-L'0';
    return towupper(c)-L'A';
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


wchar_t *wcsdup_len(const wchar_t *wstring, size_t *len_ptr) {
    if (wstring==NULL) {
        *len_ptr=0;
        return NULL;
    }
    
    size_t length = wcslen(wstring);
    *len_ptr=length;
    length++;
    wchar_t *new_wstring=malloc(length*sizeof(wchar_t));
    wcscpy(new_wstring, wstring);
    return new_wstring;
}

wchar_t *str_to_wcs(const char *string, size_t length) {
    if (string==NULL)
        return NULL;

    if (length==0)
        length=strlen(string);

    wchar_t *wstring = malloc(length*sizeof(wchar_t));
    mbstowcs(wstring, string, length);
    return wstring;
}
