#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>
#include <iso646.h>

#include "dict_tree.h"
#include "charset.h"

#ifndef LEXER_H
#define LEXER_H

#define TABSIZE 4
#define LEXER_BUFFSIZE 256
#define BUFF_GROWRATE 1.3f

extern const char *TokenType_string[];

// ∀∁∂∃∄∅∆∇∈∉∊∋∌∍∎∏∐∑−
/* TODO: add const|var, struct, enum, from, import, jumpto,
 * sqr, cube, abs, ..., Re, Im, !∈, ¬,
 * {.x=7, .y=2, [0]=1, [1]=4.5}
 * (a (== | != | < | > | <=) b)' = {a=b: ∞, 0}
**/
typedef enum {
    TT_EOF=0, TT_ERROR,          // End of file, syntax error
    EQ, NE, AEQ, NAE, REQ, NRE,     // Comparisons
    LT, GT, LE, GE,                 //
    PLUS, MINUS, MUL, TRUEDIV,      // Operations
    DIV, MOD, POW, MATMUL_AT,       //
    OR, AND, XOR,                   // Set operations
    IADD, ISUB, IMUL, ITRUEDIV,     // Self-eq
    IDIV, IMOD, IPOW, IMATMUL,      //
    IOR, IAND, IXOR,                //
    PM, SQR, CUBE, SQRT,            // Special operations
    PAREN_L, PAREN_R,               // Parenthesis
    SBRACK_L, SBRACK_R,             // Square brackets
    CBRACK_L, CBRACK_R,             // Curly braces
    DOT, COMMA, COLON, SEMICOLON,   // separators
    SET, ARROW, EXCL, ABS_BRACKET,  // Special symbols
    LENGTH, DOLLAR, FUNC_DERIV, TERNARY,
    KW_SUM, KW_PROD, KW_INT, DERIVATIVE, // Sum, product, integral, derivative
    KW_NOT, KW_OR, KW_AND,          // Boolean operations
    KW_IS, KW_IN, KW_NOT_IN, SUBSET, SUPERSET, // is, in/∈, not in/∉, ⊂, ⊃
    KW_IF, KW_ELSE, KW_SWITCH, KW_CASE,
    KW_LOOP, KW_FOR, KW_CONTINUE, KW_BREAK,
    KW_FUNC, KW_RETURN,             //
    INDENT, NEWLINE,                // Whitespaces
    IDENTIFIER,                     //
    STRING, FSTRING,                //
    REAL_INT, REAL_FLOAT, IMAG_INT, IMAG_FLOAT,
    KW_NONE, KW_TRUE, KW_FALSE, KW_NAN, KW_INF, LCHAR
} TokenType;

typedef struct {
    TokenType type;
    ushort line, col, value_size;
    wchar_t *value;
} Token;

void free_Token(Token *);
void print_Token_long(Token);
void print_Token(Token);
void print_error(Token);

typedef struct {
    FILE *file;
    wchar_t buffer[LEXER_BUFFSIZE];
    size_t pos, buffsize, line, col;
} Lexer;

Lexer *new_Lexer(FILE *file);
void free_Lexer(Lexer *);
Token next_token(Lexer *);

#endif
