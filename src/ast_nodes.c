#include "ast_nodes.h"

void free_Atom(Atom *atom) {
    if (atom==NULL)
        return;

    switch (atom->type) {
        case AT_STRING:
        case AT_FSTRING:
            free(atom->string);
    }
}

void print_Atom(Atom atom) {
    switch (atom.type) {
        case AT_CHAR:
            wprintf(L"Atom('%lc')", atom.character); break;
        case AT_STRING:
            wprintf(L"Atom(\"%ls\")", atom.string); break;
        case AT_FSTRING:
            wprintf(L"Atom($\"%ls\")", atom.string); break;
        case AT_INT:
            wprintf(L"Atom(%ld%s", atom.integer, atom.is_real? ")": "i)");
            break;
        case AT_FLOAT:
            wprintf(
                L"Atom(%lg%s", atom.floating_point,
                atom.is_real? ")": "i)"
            );
    }
}

Atom Atom_from_Token(Token token) { // frees Token
    wchar_t c, *end_string;
    int64_t integer;
    double floating_point;
    switch (token.type) {
        case KW_NONE:
            return (Atom){AT_NONE};
        case KW_TRUE:
        case KW_FALSE:
            return (Atom){AT_BOOL, .boolean=(token.type==KW_TRUE)};
        case LCHAR:
            c=token.value[0];
            free(token.value);
            return (Atom){AT_CHAR, .character=c};
        case STRING:
            return (Atom){AT_STRING, .string=token.value};
        case FSTRING:
            return (Atom){AT_FSTRING, .string=token.value};
        
        case REAL_INT:
        case IMAG_INT:
            integer=wcstoll(token.value, &end_string, 10);
            free(token.value);
            return (Atom){
                AT_INT, .is_real=(token.type==REAL_INT), .integer=integer
            };
        case REAL_FLOAT:
        case IMAG_FLOAT:
            floating_point=wcstod(token.value, &end_string);
            free(token.value);
            return (Atom){
                AT_FLOAT, .is_real=(token.type==REAL_FLOAT),
                .floating_point=floating_point
            };

        case KW_INF:
            return (Atom){AT_FLOAT, .is_real=true, .floating_point=INFINITY};
        case KW_NAN:
            return (Atom){AT_FLOAT, .is_real=true, .floating_point=NAN};
    }

    return (Atom){AT_EMPTY};
}


const ushort RANGE_START=0, RANGE_STOP=0;

Expression *new_Expression(enum ExpressionType type) {
    Expression *expr=malloc(sizeof(Expression));
    expr->type=type;
    return expr;
}

void free_Expression(Expression *expr) {
    if (expr==NULL)
        return;

    switch (expr->type) {
        case NT_ATOM:
            free_Atom(&expr->atom);
            break;
        case NT_BINOP:
        case NT_RANGE:
            free_Expression(expr->left);
            free_Expression(expr->right);
            break;
        case NT_COMP:
            for (ushort i=0; i<=expr->parameter; i++)
                free_Expression(expr->values[i]);
            free(expr->operators);
            break;
        case NT_UNARY_PREFIX:
        case NT_UNARY_POSTFIX:
            free_Expression(expr->value);
    }

    free(expr);
}

void print_expr_long(Expression *expr) {
    wprintf(L"Expression(");
    ushort param=expr->parameter;
    switch (expr->type) {
        case NT_ATOM:
            print_Atom(expr->atom);
            break;
        case NT_RANGE:
            wprintf(L"rangetype=%hu, left=", param);
            print_expr_long(expr->left);
            wprintf(L", right=");
            print_expr_long(expr->right);
            break;
        case NT_UNARY_PREFIX:
        case NT_UNARY_POSTFIX:
            wprintf(L"oper=%s, value=", TokenType_string[expr->oper]);
            print_expr_long(expr->value);
            break;
        case NT_BINOP:
            wprintf(L"left=");
            print_expr_long(expr->left);
            wprintf(L", oper=%s, right=", TokenType_string[expr->oper]);
            print_expr_long(expr->right);
            break;
        case NT_COMP:
            for (ushort i=0; i<param; i++) {
                print_expr_long(expr->values[i]);
                wprintf(L", %s, ", TokenType_string[expr->operators[i]]);
            }
            print_expr_long(expr->values[param]);
            break;
    }
    putwchar(L')');
}

void print_expr(Expression *expr) {
    ushort param=expr->parameter;
    switch (expr->type) {
        case NT_ATOM:
            print_Atom(expr->atom);
            return;
        case NT_RANGE:
            putwchar((param&RANGE_START)? L'[': L'(');
            print_expr(expr->left);
            wprintf(L"; ");
            print_expr(expr->right);
            putwchar((param&RANGE_STOP)? L']': L')');
            return;
        case NT_UNARY_PREFIX:
        case NT_UNARY_POSTFIX:
            wprintf(L"(%s ", TokenType_string[expr->oper]);
            print_expr(expr->value);
            break;
        case NT_BINOP:
            wprintf(L"(");
            print_expr(expr->left);
            wprintf(L" %s ", TokenType_string[expr->oper]);
            print_expr(expr->right);
            break;
        case NT_COMP:
            putwchar(L'(');
            for (ushort i=0; i<param; i++) {
                print_expr(expr->values[i]);
                wprintf(L" %s ", TokenType_string[expr->operators[i]]);
            }
            print_expr(expr->values[param]);
            break;
    }
    putwchar(L')');
}


Command *new_Command() {
    Command *command=malloc(sizeof(Command));
    return command;
}

void free_Command(Command *command) {
    if (command==NULL)
        return;

    free(command);
}

void print_command_long(Command *command) {
    wprintf(L"COMMAND(...long)");
}

void print_command(Command *command) {
    wprintf(L"COMMAND(...)");
}


void free_ASTNode(ASTNode *node) {
    if (node==NULL)
        return;

    switch (node->type) {
        case NT_ERROR:
            free(node->error_code);
            break;
        case NT_EXPR:
            free_Expression(node->expr);
            break;
        case NT_COMMAND:
            free_Command(node->command);
            break;
    }
}

void print_ASTNode(ASTNode node) {
    switch (node.type) {
        case NT_EOF:
            wprintf(L"EOF"); break;
        case NT_ERROR:
            wprintf(L"Syntax Error: %ls", node.error_code); break;
        case NT_EMPTY:
            wprintf(L"EMPTY"); break;
        case NT_WORD:
            wprintf(L"%s", TokenType_string[node.tt]); break;
        case NT_INDENT:
            wprintf(L"INDENT: %hu", node.indent); break;
        case NT_EXPR:
            print_expr(node.expr); break;
        case NT_COMMAND:
            print_command(node.command);
    }
}

ASTNode ASTNode_error(ASTNode node, wchar_t *error_code) {
    return (ASTNode){
        NT_ERROR, .error_code=string_copy(error_code),
        .line=node.line, .col=node.col
    };
}

ASTNode ASTNode_error_from_Token(Token token, wchar_t *error_code) {
    return (ASTNode){
        NT_ERROR, .error_code=string_copy(error_code),
        .line=token.line, .col=token.col
    };
}

ASTNode ASTNode_from_Token(Token token) { // frees Token
    ASTNode node = (ASTNode){.line=token.line, .col=token.col};
    switch (token.type) {
        case TT_EOF:
            node.type=NT_EOF;
            return node;
        case TT_ERROR:
            node.type=NT_ERROR;
            node.error_code=token.value;
            return node;
        case INDENT:
            node.type=NT_INDENT;
            node.indent=token.value_size;
            return node;
    }

    Atom atom=Atom_from_Token(token);
    if (atom.type==AT_EMPTY) {
        node.type=NT_WORD;
        node.tt=token.type;
        return node;
    }

    Expression *expr=new_Expression(NT_ATOM);
    expr->atom=atom;
    return (ASTNode){NT_EXPR, .expr=expr};
}

ASTNode ASTNode_from_expr(Expression *expr) {
    return (ASTNode){NT_EXPR, .expr=expr}; //TODO: something with this
}
