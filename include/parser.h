#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>
#include <math.h>
#include <iso646.h>

#include "lexer.h"
#include "ast_nodes.h"
#include "utils/dict_tree.h"

#define TOKEN_BUFFSIZE 16
#define MAX_CHAINLEN 256

typedef struct {
	Lexer *lexer;
	DictTree *datatypes;
	Token buffer[TOKEN_BUFFSIZE];
	size_t pos, buffsize;
} Parser;

Parser *new_Parser(FILE *file);
void free_Parser(Parser *parser);
ASTNode parse_line(Parser *parser);

#endif
