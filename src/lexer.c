#include "lexer.h"

const char *TokenType_string[] = {
    "TT_EOF", "TT_ERROR",
    "EQ", "NE", "AEQ", "NAE", "REQ", "NRE",
    "LT", "GT", "LE", "GE",
    "PLUS", "MINUS", "MUL", "TRUEDIV",
    "DIV", "MOD", "POW", "MATMUL_AT",
    "OR", "AND", "XOR",
    "IADD", "ISUB", "IMUL", "ITRUEDIV",
    "IDIV", "IMOD", "IPOW", "IMATMUL",
    "IOR", "IAND", "IXOR",
    "PM", "SQR", "CUBE", "SQRT",
    "PAREN_L", "PAREN_R",
    "SBRACK_L", "SBRACK_R",
    "CBRACK_L", "CBRACK_R",
    "DOT", "COMMA", "COLON", "SEMICOLON",
    "SET", "ARROW", "EXCL", "ABS_BRACKET",
    "LENGTH", "DOLLAR", "FUNC_DERIV", "TERNARY",
    "KW_SUM", "KW_PROD", "KW_INT", "DERIVATIVE",
    "KW_NOT", "KW_OR", "KW_AND",
    "KW_IS", "KW_IN", "KW_NOT_IN", "SUBSET", "SUPERSET",
    "KW_IF", "KW_ELSE", "KW_SWITCH", "KW_CASE",
    "KW_LOOP", "KW_FOR", "KW_CONTINUE", "KW_BREAK",
    "KW_FUNC", "KW_RETURN",
    "INDENT", "NEWLINE",
    "IDENTIFIER",
    "STRING", "FSTRING",
    "REAL_INT", "REAL_FLOAT", "IMAG_INT", "IMAG_FLOAT",
    "KW_NONE", "KW_TRUE", "KW_FALSE", "KW_NAN", "KW_INF", "LCHAR"
};

//TODO: remove \$
#define ESCAPE_CODES_SIZE 119
static const wchar_t escape_codes[ESCAPE_CODES_SIZE] = { 
    [L'"']=L'"', [L'\'']=L'\'', [L'$']=L'$', [L'\\']=L'\\',
    [L'a']=L'\a', [L'b']=L'\b', [L'f']=L'\f',
    [L'n']=L'\n', [L'r']=L'\r', [L's']=L' ', [L't']=L'\t', [L'v']=L'\v'
};

//TODO: optimize "⊂⊃"
#define SINGLE_TOKEN_SIZE L'⊃'+1
static const TokenType single_tokens[SINGLE_TOKEN_SIZE] = {
    ['#']=LENGTH, ['\'']=FUNC_DERIV, ['(']=PAREN_L, [')']=PAREN_R,
    [',']=COMMA, ['.']=DOT, [':']=COLON, [';']=SEMICOLON, ['?']=TERNARY,
    ['[']=SBRACK_L, [']']=SBRACK_R, ['{']=CBRACK_L, ['}']=CBRACK_R,
    [L'±']=PM, [L'²']=SQR, [L'³']=CUBE, [L'Ø']=KW_NAN, //TODO: emptyset
    [L'∂']=DERIVATIVE, [L'∈']=KW_IN, [L'∉']=KW_NOT_IN, [L'√']=SQRT,
    [L'∞']=KW_INF, [L'∫']=KW_INT, [L'≈']=REQ, [L'≠']=NE,
    [L'≤']=LE, [L'≥']=GE, [L'⊂']=SUBSET, [L'⊃']=SUPERSET
};

#define DOUBLE_SIZE 128
static const TokenType double_tokens[DOUBLE_SIZE][2] = {
    ['%']={MOD, IMOD}, ['@']={MATMUL_AT, IMATMUL},
    ['&']={AND, IAND}, ['^']={XOR, IXOR},
    ['=']={SET, EQ}, ['~']={AEQ, REQ}, ['<']={LT, LE}, ['>']={GT, GE}
};

const wchar_t imagchar=L'i';
const TokenType number_types[4] = {IMAG_INT, IMAG_FLOAT, REAL_INT, REAL_FLOAT};

static DictTree *keyword_tokens = NULL;

#define P(str, tt) set_word(keyword_tokens, str, (const void *)tt)

__attribute__((constructor)) void setup_keywords() {
    keyword_tokens=new_DictTree(); {
        P(L"None", KW_NONE), P(L"true", KW_TRUE), P(L"false", KW_FALSE),
        P(L"NaN", KW_NAN), P(L"Inf", KW_INF), P(L"sqrt", SQRT),
        P(L"subset", SUBSET), P(L"superset", SUPERSET),
        P(L"sum", KW_SUM), P(L"prod", KW_PROD),
        P(L"Σ",   KW_SUM), P(L"Π",    KW_PROD), P(L"int", KW_INT),
        P(L"not", KW_NOT), P(L"and", KW_AND), P(L"or", KW_OR),
        P(L"is", KW_IS), P(L"in", KW_IN),
        P(L"if", KW_IF), P(L"else", KW_ELSE),
        P(L"switch", KW_SWITCH), P(L"case", KW_CASE),
        P(L"loop", KW_LOOP), P(L"for", KW_FOR),
        P(L"continue", KW_CONTINUE), P(L"break", KW_BREAK),
        P(L"func", KW_FUNC), P(L"return", KW_RETURN);
    };
}

__attribute__((destructor)) void teardown_keywords() {
    free_DictTree(keyword_tokens);
}

#undef P


void free_Token(Token *token) {
    if (token!=NULL and token->value!=NULL)
        free(token->value);
}

void print_Token_long(Token token) {
    TokenType tt=token.type;
    wprintf(L"Token(%s, ", TokenType_string[tt]);
    if (tt==INDENT)
        wprintf(L"%hu, ", token.value_size);
    else if (token.value!=NULL)
        wprintf(L"\"%ls\", ", token.value);

    wprintf(L"line=%hu, col=%hu)", token.line, token.col);
}

void print_Token(Token token) {
    TokenType tt=token.type;
    wprintf(L"%s", TokenType_string[tt]);
    if (tt==INDENT)
        wprintf(L": %hu", token.value_size);
    else if (token.value!=NULL)
        wprintf(L": \"%ls\"", token.value);
}

void print_error(Token token) {
    wprintf(L"Syntax Error");
    if (token.value!=NULL)
        wprintf(L": %ls", token.value);
}


Lexer *new_Lexer(FILE *file) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->file=file;
    lexer->buffsize=0;
    lexer->pos=0;
    
    lexer->line=1;
    lexer->col=0;
    return lexer;
}

void free_Lexer(Lexer *lexer) {
    if (lexer==NULL)
        return;
    
    free(lexer);
}

Token make_Token(Lexer *lexer, TokenType type, const wchar_t *value) {
    Token token = (Token){type, .value=NULL};
    size_t value_size = 0;
    if (type==INDENT)
        value_size=(ushort)value;
    else if (value!=NULL)
        token.value=string_copy_len(value, &value_size);
    
    token.value_size=value_size;
    token.line=lexer->line;
    token.col=lexer->col+lexer->pos;
    return token;
}

static bool next_line(Lexer *lexer) {
    wchar_t *buffer=lexer->buffer;
    size_t buffsize=lexer->buffsize;
    
    lexer->col+=buffsize; lexer->pos=0;
    if (buffsize>=1 && buffer[buffsize-1]==L'\n') {
        lexer->line++; //TODO: test if buffer always ends in \n.
        lexer->col=0;
    }
    
    if (fgetws(buffer, LEXER_BUFFSIZE, lexer->file)==NULL) {
        lexer->buffsize=0;
        return false;
    }
    buffsize=wcsnlen(buffer, LEXER_BUFFSIZE);
    lexer->buffsize=buffsize;
    return true;
}

static wchar_t curr_char(Lexer *lexer) {
    return lexer->buffer[lexer->pos];
}

static wchar_t next_char(Lexer *lexer) {
    if (++lexer->pos >= lexer->buffsize && !next_line(lexer))
        return L'\0';
    return lexer->buffer[lexer->pos];
}

static void prev_char(Lexer *lexer) {
    if (lexer->pos<=0) {
        perror("Cannot roll back to previous char when pos <= 0");
        return;
    }
    lexer->pos--;
}


static Token handle_excl(Lexer *lexer) {
    switch (next_char(lexer)) {
        case L'=': return make_Token(lexer, NE, NULL);
        case L'≈': return make_Token(lexer, NRE, NULL);
        case L'~':
            if (next_char(lexer)==L'=')
                return make_Token(lexer, NRE, NULL);
            prev_char(lexer);
            return make_Token(lexer, NAE, NULL);
        case L'∈': return make_Token(lexer, KW_NOT_IN, NULL);
    }

    prev_char(lexer);
    return make_Token(lexer, EXCL, NULL);
}

static Token handle_string(Lexer *lexer) {
    size_t i, buffsize=32;
    wchar_t *buffer=calloc(buffsize, sizeof(wchar_t));

    wchar_t c=next_char(lexer);
    for (i=0; c!=L'"'; c=next_char(lexer)) {
        if (c==L'\0') {
            free(buffer);
            return make_Token(lexer, TT_ERROR, L"Unexpected EOF");
        } //TODO: handle backslash
        
        if (i>=(buffsize-1)) {
            buffsize*=BUFF_GROWRATE;
            buffer=realloc(buffer, buffsize*sizeof(wchar_t));
        }
        buffer[i++]=c;
    }

    buffer[i++]=L'\0';
    return make_Token(lexer, STRING, buffer);
}

static Token handle_dollar(Lexer *lexer) {
    Token token;
    wchar_t c, c0=next_char(lexer);
    switch (c0) {
        case L'"':
            token=handle_string(lexer); //TODO: fstring parsing
            if (token.type!=TT_ERROR)
                token.type=FSTRING;
            return token;
        case L'\'':
            c0=next_char(lexer);
            if (c0==L'\\') { //TODO: handle hex
                c0=next_char(lexer);
                c=(c0<ESCAPE_CODES_SIZE)? escape_codes[c0]: L'\0';
                if (c==L'\0')
                    return make_Token(lexer, TT_ERROR, L"Unknown escape sequence");
            } else if (c==L'\'')
                return make_Token(lexer, TT_ERROR, L"Empty character literal");
            else
                c=c0;

            if (next_char(lexer)!=L'\'')
                return make_Token(lexer, TT_ERROR, L"Unclosed character literal");
            
            c0=L'\0';
            return make_Token(lexer, LCHAR, &c);
    }

    prev_char(lexer);
    return make_Token(lexer, DOLLAR, NULL);
}

static Token handle_star(Lexer *lexer) {
    switch (next_char(lexer)) {
        case L'*':
            if (next_char(lexer)==L'=')
                return make_Token(lexer, IPOW, NULL);

            prev_char(lexer);
            return make_Token(lexer, POW, NULL);
        case L'=':
            return make_Token(lexer, IMUL, NULL);
    }

    prev_char(lexer);
    return make_Token(lexer, MUL, NULL);
}

static Token handle_plus(Lexer *lexer) {
    switch (next_char(lexer)) {
        case L'=': return make_Token(lexer, IADD, NULL);
        case L'-': return make_Token(lexer, PM, NULL);
    }

    prev_char(lexer);
    return make_Token(lexer, PLUS, NULL);
}

static Token handle_minus(Lexer *lexer) {
    switch (next_char(lexer)) {
        case L'=': return make_Token(lexer, ISUB, NULL);
        case L'>': return make_Token(lexer, ARROW, NULL);
    }

    prev_char(lexer);
    return make_Token(lexer, MINUS, NULL);
}

static Token handle_slash(Lexer *lexer) {
    switch (next_char(lexer)) {
        case L'/':
            if (next_char(lexer)==L'=')
                return make_Token(lexer, IDIV, NULL);

            prev_char(lexer);
            return make_Token(lexer, DIV, NULL);
        case L'=':
            return make_Token(lexer, ITRUEDIV, NULL);
    }

    prev_char(lexer);
    return make_Token(lexer, TRUEDIV, NULL);
}

static Token handle_whitespace(Lexer *lexer);

static Token handle_backslash(Lexer *lexer) { //TODO
    wchar_t c=next_char(lexer);
    if (c==L'\\') {
        for (; c!='\n'; c=next_char(lexer)) {
            if (c==L'\0')
                return make_Token(lexer, TT_EOF, NULL);
        }
        return make_Token(lexer, NEWLINE, NULL);
    }

    prev_char(lexer);
    Token token; token.type=TT_EOF;
    for (; token.type!=NEWLINE; token=handle_whitespace(lexer)) {
        c=next_char(lexer);
        if (c==L'\0')
            return make_Token(lexer, TT_ERROR, L"Unexpected EOF");
        if (!valid_whitespace(c))
            return make_Token(lexer, TT_ERROR, L"Non-whitespace character after backslash");
    }

    return make_Token(lexer, INDENT, (void *)1);
}

static Token handle_pipe(Lexer *lexer) {
    switch (next_char(lexer)) {
        case L'|': return make_Token(lexer, ABS_BRACKET, NULL);
        case L'=': return make_Token(lexer, IOR, NULL);
    }

    prev_char(lexer);
    return make_Token(lexer, OR, NULL);
}

static Token handle_whitespace(Lexer *lexer) {
    wchar_t c=curr_char(lexer);
    if (c==L'\n')
        return make_Token(lexer, NEWLINE, NULL);
    
    ushort count=0;
    for (; c==L' ' or c==L'\t'; c=next_char(lexer))
        count+=(c==L'\t')? TABSIZE: 1;
    
    prev_char(lexer);
    return make_Token(lexer, INDENT, (void *)(count/TABSIZE));
}

static Token handle_number(Lexer *lexer) {
    size_t i=0, buffsize=16;
    wchar_t *buffer=calloc(buffsize, sizeof(wchar_t));
    buffer[0]=curr_char(lexer);
    
    wchar_t c=next_char(lexer);
    bool is_real=true, is_float=false;
    for (; c!=L'\0'; c=next_char(lexer)) {
        if (c==L'.') {
            if (is_float)
                return make_Token(lexer, TT_ERROR, L"Unexpected second dot");
            is_float=true;
        } else if (c==imagchar) {
            is_real=false;
            goto end;
        } else if (!valid_digit(c)) {
            prev_char(lexer);
            goto end;
        }

        if (++i>=(buffsize-1)) {
            buffsize*=BUFF_GROWRATE;
            buffer=realloc(buffer, buffsize*sizeof(wchar_t));
        }
        buffer[i]=c;
    }

    end:
    buffer[++i]=L'\0';
    Token token = make_Token(lexer, number_types[2*is_real+is_float], buffer);
    free(buffer);
    return token;
}

static Token handle_identifier(Lexer *lexer) {
    size_t i=0, buffsize=16;
    wchar_t *buffer = calloc(buffsize, sizeof(wchar_t));
    buffer[0]=curr_char(lexer);
    
    wchar_t c=next_char(lexer);
    for (; c!=L'\0'; c=next_char(lexer)) {
        if (!valid_alnum(c)) {
            prev_char(lexer);
            break;
        }

        if (++i>=(buffsize-1)) {
            buffsize*=BUFF_GROWRATE;
            buffer=realloc(buffer, buffsize*sizeof(wchar_t));
        }
        buffer[i]=c;
    }

    buffer[++i]=L'\0';
    TokenType tt = (TokenType)get_word(keyword_tokens, buffer);
    if (tt!=TT_EOF) {
        free(buffer);
        return make_Token(lexer, tt, NULL);
    }

    Token token = make_Token(lexer, IDENTIFIER, buffer);
    free(buffer);
    return token;
}


Token next_token(Lexer *lexer) {
    wchar_t c=next_char(lexer);
    if (c==L'\0')
        return make_Token(lexer, TT_EOF, NULL);
    
    if (valid_whitespace(c))
        return handle_whitespace(lexer);
    if (valid_digit(c))
        return handle_number(lexer);
    if (valid_alpha(c))
        return handle_identifier(lexer);

    // Handle single symbol tokens
    TokenType tt = (c<SINGLE_TOKEN_SIZE)? single_tokens[c]: TT_EOF;
    if (tt!=TT_EOF)
        return make_Token(lexer, tt, NULL);

    // Handle symbol-equals pair
    const TokenType *pair = (c<DOUBLE_SIZE)? double_tokens[c]: NULL;
    if (pair!=NULL && *pair!=TT_EOF) {
        if (next_char(lexer)==L'=')
            return make_Token(lexer, pair[1], NULL);

        prev_char(lexer);
        return make_Token(lexer, pair[0], NULL);
    }

    // Handle special symbols
    switch (c) { //TODO: handle_dot: ..., .05
        case L'!': return handle_excl(lexer);
        case L'"': return handle_string(lexer);
        case L'$': return handle_dollar(lexer);
        case L'*': return handle_star(lexer);
        case L'+': return handle_plus(lexer);
        case L'-': return handle_minus(lexer);
        case L'/': return handle_slash(lexer);
        case L'\\': return handle_backslash(lexer);
        case L'|': return handle_pipe(lexer);
    }

    return make_Token(lexer, TT_ERROR, L"Unknown char"); //TODO: handle char format
}
