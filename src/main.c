#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "rt_value.h"
#include "charset.h"

int lex_shell(FILE *file) {
    Lexer *lexer = new_Lexer(file);

    Token token = next_token(lexer);
    for (; token.type!=TT_EOF; token=next_token(lexer)) {
        print_Token(token);
        free_Token(&token);
        wprintf(L", ");
    }
    print_Token(token);
    wprintf(L".\n\n");

    free_Lexer(lexer);
    return 0;
}

int exec_shell(FILE *file) {
    Parser *parser=new_Parser(file);
    ASTNode node;
    RTExpr *rt_expr;

    while (true) {
        wprintf(L"\n\nms> ");
        node=parse_line(parser);
        if (node.type==NT_EOF)
            break;

        print_ASTNode(node);
        rt_expr=eval_ASTNode(node);
        
        wprintf(L"\n! ");
        print_RTExpr(rt_expr);
    }
    putwchar(L'\n');

    free_Parser(parser);
    return 0;
}

int exec_file() {
    FILE *file = fopen("docs/input.txt", "r");
    if (file==NULL) {
        perror("Error opening file");
        return 1;
    }

    exec_shell(file);

    if (fclose(file)==EOF) {
        perror("Error closing file");
        return 1;
    }
    return 0;
}

#define PRINT_SIZE(type) wprintf(L"sizeof(" #type L") = %ld\n", sizeof(type))
static void print_size_info() {
    PRINT_SIZE(wchar_t);
    PRINT_SIZE(long);
    PRINT_SIZE(double);
    PRINT_SIZE(TokenType);
    PRINT_SIZE(Token);
    PRINT_SIZE(Atom);
    PRINT_SIZE(Expression);
    PRINT_SIZE(Command);
    PRINT_SIZE(ASTNode);
    PRINT_SIZE(RTValue);
    PRINT_SIZE(RTExpr);
}
#undef PRINT_SIZE

int main(int argc, char *argv[]) {
    // Greek letters support for some terminals:
    setlocale(LC_CTYPE, "el_GR.UTF-8");

    putwchar(L'\n');
    if (argc>=2 and !strncmp(argv[1], "-s", 3)) //TODO: interactive mode -i flag
        print_size_info();

    wprintf(
        L"\n\t\t\t\t= MathScript Interpreter =\n"
        L"\nAllowed symbols:\n"
        L" !\"#$%&'()*+,-./:;<=>?@[\\]^{|}~±²³Ø∂∈∉√∞∫≈≠≤≥⊂⊃\n"
    );
    for (wchar_t c=L' '; c<1000; c++) {
        if (valid_alnum(c)) putwchar(c);
    }
    
    return exec_file();
}
