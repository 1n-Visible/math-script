#include "parser.h"

#define SWAP_PTR(ptr1, ptr2) {void *tmp_ptr=ptr1; ptr1=ptr2; ptr2=tmp_ptr;}
//TODO: meta-sets: (13 in Primes) -> true; (0.5 in Z) -> false, ⟂
//TODO: discrete ranges: sum [n=1; N] (...) | sum (n in [2; N]) (...)

Parser *new_Parser(FILE *file) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer=new_Lexer(file);
    parser->datatypes=new_DictTree();

    parser->buffsize=0; parser->pos=0;
    return parser;
}

void free_Parser(Parser *parser) {
    if (parser==NULL)
        return;

    free_Lexer(parser->lexer);
    free_DictTree(parser->datatypes);
    free(parser);
}


/* === PARSING FUNCTIONS === */
/* Operator precedence:
DOT_ACCESS
array[INDEX]
LENGTH = #array
SQR ^ CUBE [||ABS|| ^ FACTORIAL]
POW
SQRT | {PLUS ^ MINUS ^ PM}
MUL, TRUEDIV, MATMUL, DIV, MOD
PLUS, MINUS, PM, [MP]
AND, XOR, OR
EQ, NE, AEQ, NAE, REQ, NRE, LT, GT, LE, GE
, KW_IS, KW_IN, KW_NOT_IN, SUBSET, SUPERSET
KW_NOT, KW_AND, KW_OR # Ternary??
*/

#define FREE_BUFF() free(which_buffer? new_buffer: buffer)
#define parse_binop(LABEL, MATCH) { \
    Expression *expr, *left; \
    for (i=0, new_buffsize=0; i<buffsize; i++) { \
        node=buffer[i]; \
        if (node.type!=NT_EXPR) goto LABEL; \
        \
        left=node.expr; \
        for (i++; i<buffsize-1; i+=2) { \
            node=buffer[i]; \
            if (node.type!=NT_WORD) {i--; break;} \
            \
            TokenType type=node.tt; \
            switch (type) { \
                MATCH \
                node=buffer[i+1]; \
                if (node.type!=NT_EXPR) { \
                    node=ASTNode_error(node, L"Expected expression, got ..."); \
                    goto LABEL; \
                } \
                expr=new_Expression(NT_BINOP); \
                expr->left=left; \
                expr->oper=type; \
                expr->right=node.expr; \
                left=expr; \
                continue; \
            } \
            i--; break; \
        } \
        node=(ASTNode){NT_EXPR, .expr=left}; \
        LABEL: \
        new_buffer[new_buffsize++]=node; \
        if (node.type==NT_ERROR) break; \
    } \
    node=new_buffer[new_buffsize-1]; \
    if (new_buffsize==1) { \
        FREE_BUFF(); return node; \
    } else if (node.type==NT_ERROR) { \
        new_buffsize--; i=0; \
        FREE_BUFF(); return node; \
    } else { \
        node=new_buffer[0]; i=1; \
    } \
    SWAP_PTR(buffer, new_buffer); \
    which_buffer=!which_buffer; \
}

static ASTNode parse_expr(ASTNode *buffer, size_t buffsize) {
    size_t i, j, new_buffsize;
    ASTNode node, *new_buffer=calloc(buffsize, sizeof(ASTNode));
    bool which_buffer=true;

    parse_binop(
        push_mul, case MUL: case TRUEDIV:
        case DIV: case MOD: case MATMUL_AT:);
    parse_binop(push_add, case PLUS: case MINUS: case PM:);
    
    FREE_BUFF();
    return node;
}

#undef FREE_BUFF
#undef parse_binop

static ASTNode *scan_until(Parser *parser, TokenType termin, size_t *return_size,
                           short *index, ushort *rangetype);

static ASTNode parse_parenth(Parser *parser) {
    short index;
    ushort rangetype;
    size_t i, buffsize;
    ASTNode node, *buffer=scan_until(
        parser, PAREN_R, &buffsize, &index, &rangetype
    );

    node=buffer[buffsize-1];
    if (node.type==NT_ERROR) {
        buffsize--;
        goto end;
    }

    if (index==-1) {
        node=parse_expr(buffer, buffsize);
        goto end;
    }

    ASTNode left=parse_expr(buffer, index);
    if (left.type==NT_ERROR) {
        node=left; goto end;
    }

    ASTNode right=parse_expr(buffer+index+1, buffsize-index-1);
    if (right.type==NT_ERROR) {
        free_ASTNode(&left);
        node=right; goto end;
    }

    Expression *expr=new_Expression(NT_RANGE);
    expr->parameter=rangetype;
    expr->left=left.expr;
    expr->right=right.expr;
    node=(ASTNode){NT_EXPR, .expr=expr};

    end:
    free(buffer);
    return node;
}

static ASTNode parse_brackets(Parser *parser) {
    return ASTNode_error_from_Token(next_token(parser->lexer), L"Unexpected '['");
}

static ASTNode parse_braces(Parser *parser) {
    return ASTNode_error_from_Token(next_token(parser->lexer), L"Unexpected '{'");
}

//TODO: rename
static ASTNode *scan_until(Parser *parser, TokenType termin,
                           size_t *return_size, short *index, ushort *rangetype) {
    size_t i=0, buffsize=16;
    ASTNode node, *buffer=calloc(buffsize, sizeof(ASTNode));
    *index=-1;
    
    Token token; bool is_range=false;
    while (true) {
        token=next_token(parser->lexer);
        switch (token.type) {
            case TT_EOF: node=ASTNode_error_from_Token(token, L"Unexpected EOF"); break;
            case NEWLINE:
            case INDENT:
                continue;
            case PAREN_L:  node=parse_parenth(parser); break;
            case SBRACK_L: node=parse_brackets(parser); break;
            case CBRACK_L: node=parse_braces(parser); break;
            case PAREN_R:
            case SBRACK_R:
                if (is_range) {
                    node=ASTNode_from_Token(token);
                    *rangetype=(termin==SBRACK_R)*RANGE_START |
                            (token.type==SBRACK_R)*RANGE_STOP;
                    goto end;
                }
                if (token.type==termin) goto end;
                node=ASTNode_error_from_Token(token, L"Unexpected closing brackets");
                break;
            case CBRACK_R:
                node=ASTNode_error_from_Token(token, L"Unexpected '}'");
                break;
            case SEMICOLON: // [0; 1) notation
                if (is_range) {
                    node=ASTNode_error_from_Token(
                        token, L"Unexpected semicolon in range notation"
                    ); break;
                }
                *index=i;
                is_range=true;
            default:
                node=ASTNode_from_Token(token);
        }

        if (i>=buffsize) {
            buffsize*=BUFF_GROWRATE;
            buffer=realloc(buffer, buffsize*sizeof(ASTNode));
        }
        buffer[i++]=node;
        if (node.type==NT_ERROR)
            break;
    }

    end:
    *return_size=i;
    return buffer;
}

#define _BRACK_ERR(errcode) \
    node=ASTNode_error_from_Token(token, L"Unexpected " errcode); break;

ASTNode parse_line(Parser *parser) {
    size_t i=0, buffsize=16;
    ASTNode *buffer=calloc(buffsize, sizeof(ASTNode));
    ASTNode node=(ASTNode){NT_EMPTY};
    
    Token token;
    while ((token=next_token(parser->lexer)).type!=NEWLINE) {
        switch (token.type) {
            case INDENT:
                continue;
            case PAREN_L:
                node=parse_parenth(parser);
                break;
            case SBRACK_L:
                node=parse_brackets(parser);
                break;
            case CBRACK_L:
                node=parse_braces(parser);
                break;
            case PAREN_R:  _BRACK_ERR(L"')'");
            case SBRACK_R: _BRACK_ERR(L"']'");
            case CBRACK_R: _BRACK_ERR(L"'}'");
            default:
                node=ASTNode_from_Token(token);
        }

        if (node.type<=NT_ERROR)
            break;
        if (i>=buffsize) {
            buffsize*=BUFF_GROWRATE;
            buffer=realloc(buffer, buffsize*sizeof(ASTNode));
        }
        buffer[i++]=node;
    }

    buffsize=i;
    if (node.type<=NT_EMPTY)
        goto end;

    wprintf(L"buffer: ");
    print_ASTNode(buffer[0]);
    for (i=1; i<buffsize; i++) {
        wprintf(L", ");
        print_ASTNode(buffer[i]);
    }
    wprintf(L".\n");
    
    node=parse_expr(buffer, buffsize);

    end:
    free(buffer);
    return node;
}

#undef _BRACK_ERR
