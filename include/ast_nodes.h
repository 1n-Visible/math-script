#ifndef AST_NODES_H
#define AST_NODES_H

#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>
#include <math.h>

#include "lexer.h"

/* === ATOM === */
enum AtomType {
    AT_ERROR=-1, AT_EMPTY, AT_IDENTIFIER, AT_NONE, AT_BOOL,
    AT_CHAR, AT_STRING, AT_FSTRING, AT_INT, AT_FLOAT
};

typedef struct { // TODO: AT_IDENTIFIER, copy lengths
    enum AtomType type;
    bool boolean, is_real;
    union {
        int identifier_size, string_size;
    };
    union {
        wchar_t *identifier, character, *string;
        int64_t integer;
        double floating;
    };
} Atom;

void free_Atom(Atom *atom);
void print_Atom(Atom atom);

extern const ushort RANGE_START, RANGE_STOP;

typedef enum OperType {
    UNARY_LENGTH=1, UNARY_ABS, UNARY_SQR, UNARY_CUBE, UNARY_FACTORIAL,
    BINOP_POW, UNARY_SQRT,
    BINOP_MUL, BINOP_TRUEDIV, BINOP_MATMUL, BINOP_DIV, BINOP_MOD,
    UNARY_POS, UNARY_NEG, UNARY_PM,
    BINOP_ADD, BINOP_SUB, BINOP_PM,
    BINOP_AND, BINOP_XOR, BINOP_OR,
    COMP_EQ, COMP_NE, COMP_AEQ, COMP_NAE, COMP_REQ, COMP_NRE,
    COMP_LT, COMP_GT, COMP_LE, COMP_GE,
    COMP_IS, COMP_IN, COMP_NOT_IN, COMP_SUBSET, COMP_SUPERSET
} OperType;

extern const wchar_t *operators_string[];
extern const char *OperType_string[];

typedef struct Expression Expression;

/* === EXPRESSIONS === */
enum ExpressionType {
    NT_ATOM, NT_UNARY_PREFIX, NT_UNARY_POSTFIX,
    NT_BINOP, NT_COMP,
    NT_RANGE, NT_SUM, NT_PROD, NT_INT
};

struct Expression {
    enum ExpressionType type;
    OperType oper;      // unary/binary operator type
    ushort parameter;    // chain length | range type
    Atom atom;

    OperType *operators;// ==, !=, <=, >= ...chains
    Expression *value, *left, *right, **values;
};

Expression *new_Expression(enum ExpressionType type);
void free_Expression(Expression *expr);
void print_expr_long(Expression *expr);
void print_expr(Expression *expr);


typedef struct ASTNode ASTNode;

/* === COMMANDS === */
enum CommandType {
    NT_SET, NT_IF, NT_FOR, NT_LOOP, NT_FUNC
};

typedef struct {
    enum CommandType type;
    Expression *start, *condition, *step;
    ASTNode *commands;
} Command;

Command *new_Command();
void free_Command(Command *command);
void print_command_long(Command *command);
void print_command(Command *command);


/* === AST NODE === */
typedef enum {
    NT_EOF=0, NT_ERROR, NT_EMPTY,
    NT_TOKEN, NT_INDENT, NT_EXPR, NT_COMMAND
} ASTNodeType;

struct ASTNode {
    ASTNodeType type;
    union {
        wchar_t *error_code;
        TokenType tt;
        ushort indent;
        Expression *expr;
        Command *command;
    };
    size_t line, col;
};

void free_ASTNode(ASTNode *node);
void print_ASTNode(ASTNode node);

ASTNode ASTNode_error(ASTNode, wchar_t *error_code);
ASTNode ASTNode_error_from_Token(Token, wchar_t *error_code);
ASTNode ASTNode_from_Token(Token); // frees Token

#endif
