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
SQRT
PLUS ^ MINUS ^ PM
MUL, TRUEDIV, MATMUL, DIV, MOD
PLUS, MINUS, PM
AND, XOR, OR
EQ, NE, AEQ, NAE, REQ, NRE, LT, GT, LE, GE
, KW_IS, KW_IN, KW_NOT_IN, SUBSET, SUPERSET
KW_NOT, KW_AND, KW_OR # Ternary??
*/

static const OperType OperType_unary_prefix[] = {
    [PLUS]=UNARY_POS, [MINUS]=UNARY_NEG, [PM]=UNARY_PM
};

static const bool OperType_unary_prohibit[] = {
    [PLUS]=true, [MINUS]=true, [PM]=true, [MUL]=true, [TRUEDIV]=true,
    [DIV]=true, [MOD]=true, [POW]=true, [MATMUL]=true
};

static const OperType OperType_muldiv[] = {
    [MUL]=BINOP_MUL, [TRUEDIV]=BINOP_TRUEDIV, [MATMUL]=BINOP_MATMUL,
    [DIV]=BINOP_DIV, [MOD]=BINOP_MOD
};

static const OperType OperType_addsub[] = {
    [PLUS]=BINOP_ADD, [MINUS]=BINOP_SUB, [PM]=BINOP_PM
};

static const ushort lut_muldiv_len=MOD+1, lut_addsub_len=PM+1; // TODO: rename

static size_t parse_sqrt(const ASTNode *buffer, ASTNode *new_buffer,
                         size_t buffsize) {
    size_t i, j;
    for (i=0, j=0; i<buffsize; i++) {
        ASTNode node=buffer[i];
        if (node.type!=NT_TOKEN or node.tt!=SQRT)
            goto push;

        if (i==buffsize-1) {
            node=ASTNode_error(node, L"unexpected end of input after √");
            goto push;
        }

        node=buffer[++i];
        if (node.type!=NT_EXPR) {
            node=ASTNode_error(node, L"expected expression after √, got ...");
            goto push;
        }

        Expression *expr=new_Expression(NT_UNARY_PREFIX);
        expr->oper=UNARY_SQRT;
        expr->value=node.expr;
        node.expr=expr;

        push:
        new_buffer[j++]=node;
        if (node.type==NT_ERROR) break;
    }

    return j;
}

static size_t parse_unary_prefix(const ASTNode *buffer, ASTNode *new_buffer,
                                 size_t buffsize) {
    size_t i, j;
    for (i=0, j=0; i<buffsize; i++) {
        ASTNode node=buffer[i];
        if (node.type!=NT_TOKEN)
            goto push;

        OperType optype = ((node.tt<lut_addsub_len)?
                           OperType_unary_prefix[node.tt]: 0);
        if (!optype) goto push;
        if (i>0) {
            node=buffer[i-1];
            if (node.type==NT_EXPR) {
                node=buffer[i];
                goto push;
            }
            if (node.type==NT_TOKEN and OperType_unary_prohibit[node.tt]) {
                node=ASTNode_error(buffer[i], L"plus/minus after operation");
                goto push;
            }
        }

        node=buffer[++i];
        if (node.type!=NT_EXPR) {
            node=ASTNode_error(node, L"expected expression after √, got ...");
            goto push;
        }

        Expression *expr=new_Expression(NT_UNARY_PREFIX);
        expr->oper=optype;
        expr->value=node.expr;
        node.expr=expr;

        push:
        new_buffer[j++]=node;
        if (node.type==NT_ERROR) break;
    }

    return j;
}

// Iterate over buffer and write parsed version into new_buffer
static size_t parse_binop(
            const ASTNode *buffer, ASTNode *new_buffer, size_t buffsize,
            const OperType *oper_lut, ushort lut_len
    ) {
    ASTNode node;
    Expression *expr, *left;

    size_t i, j;
    for (i=0, j=0; i<buffsize; i++) {
        node=buffer[i];
        if (node.type!=NT_EXPR)
            goto push;

        left=node.expr; // Construct chain of operators
        for (i++; i<buffsize-1; i+=2) {
            node=buffer[i];
            OperType optype = (node.tt<lut_len)? oper_lut[node.tt]: 0;
            if (node.type!=NT_TOKEN or !optype) {
                i--; break;
            }

            node=buffer[i+1];
            if (node.type!=NT_EXPR) {
                node=ASTNode_error(node, L"Expected expression, got ...");
                goto push;
            }

            expr=new_Expression(NT_BINOP);
            expr->left=left;
            expr->oper=optype;
            expr->right=node.expr;
            left=expr;
        }
        node=(ASTNode){NT_EXPR, .expr=left};

        push:
        new_buffer[j++]=node;
        if (node.type==NT_ERROR) break;
    }

    return j;
}

#define SWAP_BUFF() { \
    SWAP_PTR(buffer, new_buffer); \
    size_t tmp=buffsize; buffsize=new_buffsize; new_buffsize=tmp; \
    which_buffer = not which_buffer; \
    \
    node=buffer[buffsize-1]; \
    if (buffsize==1) { \
        free(which_buffer? new_buffer: buffer); \
        return node; \
    } else if (node.type==NT_ERROR) { \
        buffsize--; \
        free(which_buffer? new_buffer: buffer); \
        return node; \
    } else node=buffer[0]; \
}

// TODO: free Expression
#define PARSE_BINOP(oper_lut, lut_len) { \
    new_buffsize=parse_binop(buffer, new_buffer, buffsize, oper_lut, lut_len); \
    SWAP_BUFF() \
}

static ASTNode parse_expr(ASTNode *buffer, size_t buffsize) {
    size_t new_buffsize;
    ASTNode node, *new_buffer=calloc(buffsize, sizeof(ASTNode));
    bool which_buffer=true;

    new_buffsize=parse_sqrt(buffer, new_buffer, buffsize);
    SWAP_BUFF()
    new_buffsize=parse_unary_prefix(buffer, new_buffer, buffsize);
    SWAP_BUFF()
    PARSE_BINOP(OperType_muldiv, lut_muldiv_len)
    PARSE_BINOP(OperType_addsub, lut_addsub_len)

    free(which_buffer? new_buffer: buffer);
    return node;
}

#undef SWAP_BUFF
#undef PARSE_BINOP

static ASTNode *scan_inbrackets(
    Parser *parser, TokenType termin, size_t *return_size,
    short *index, ushort *rangetype
);

static ASTNode parse_parenth(Parser *parser) {
    short index; ushort rangetype;
    size_t i, buffsize;
    ASTNode node, *buffer=scan_inbrackets(
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

static ASTNode *scan_inbrackets(
        Parser *parser, TokenType termin, size_t *return_size,
        short *index, ushort *rangetype
    ) {
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

static void print_ASTbuffer(ASTNode *buffer, size_t buffsize) {
    wprintf(L"buffer: ");
    print_ASTNode(buffer[0]);
    for (size_t i=1; i<buffsize; i++) {
        wprintf(L", ");
        print_ASTNode(buffer[i]);
    }
    wprintf(L".\n");
}

#define _BRACK_ERR(errormsg) \
    node=ASTNode_error_from_Token(token, L"Unexpected " errormsg); break;

ASTNode parse_line(Parser *parser) {
    size_t i=0, buffsize=16;
    ASTNode *buffer=calloc(buffsize, sizeof(ASTNode));
    ASTNode node=(ASTNode){NT_EMPTY};

    Token token;
    while (token=next_token(parser->lexer), token.type!=NEWLINE) {
        switch (token.type) {
            case INDENT:
                continue;
            case PAREN_L:
                node=parse_parenth(parser); break;
            case SBRACK_L:
                node=parse_brackets(parser); break;
            case CBRACK_L:
                node=parse_braces(parser); break;
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
    node=parse_expr(buffer, buffsize);

    end:
    free(buffer);
    return node;
}

#undef _BRACK_ERR
