#include "lexer.h"

const char *TokenType_string[] = {
	"TT_EOF", "TT_ERROR",
	"EQ", "NE", "AEQ", "NAE", "REQ", "NRE",
	"LT", "GT", "LE", "GE",
	"PLUS", "MINUS", "MUL", "DIV",
	"REMDIV", "MOD", "POW", "MATMUL",
	"OR", "AND", "XOR",
	"IADD", "ISUB", "IMUL", "IDIV",
	"IREMDIV", "IMOD", "IPOW", "IMATMUL",
	"IOR", "IAND", "IXOR",
	"PM", "SQR", "CUBE", "SQRT",
	"PAREN_L", "PAREN_R",
	"SBRACK_L", "SBRACK_R",
	"CBRACK_L", "CBRACK_R",
	"DOT", "COMMA", "COLON", "SEMICOLON",
	"SET", "ARROW", "EXCL", "ABS_BRACKET",
	"LENGTH", "DOLLAR", "FUNC_DERIV", "TERNARY",
	"KW_SUM", "KW_PROD", "KW_INT", "DERIVATIVE",
	"KW_NONE", "KW_TRUE", "KW_FALSE", "KW_NAN", "KW_INF",
	"KW_NOT", "KW_OR", "KW_AND",
	"KW_IS", "KW_IN", "KW_NOT_IN", "SUBSET", "SUPERSET",
	"KW_IF", "KW_ELSE", "KW_SWITCH", "KW_CASE",
	"KW_LOOP", "KW_FOR", "KW_CONTINUE", "KW_BREAK",
	"KW_FUNC", "KW_RETURN",
	"INDENT", "NEWLINE",
	"REAL_INT", "REAL_FLOAT", "IMAG_INT", "IMAG_FLOAT",
	"IDENTIFIER", "LCHAR", "STRING", "FSTRING"
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

const wchar_t imagchar=L'i';
const TokenType number_types[4] = {IMAG_INT, IMAG_FLOAT, REAL_INT, REAL_FLOAT};

static DictTree *keyword_tokens = NULL;
static HashMap *symbol_tokens = NULL;

#define P(str, tt) set_word(keyword_tokens, str, (const void *)tt)
#define Q(str, tt) HashMap_set(symbol_tokens, str, (const void *)tt)

__attribute__((constructor)) void setup_token_luts() {
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

	symbol_tokens=new_HashMap(10, stdcomp_wstring, stdhash_wstring); {
		Q(L"==", EQ), Q(L"!=", NE), Q(L"≠", NE), Q(L"~", AEQ), Q(L"!~", NAE),
		Q(L"~=", REQ), Q(L"≈", REQ), Q(L"!~=", NRE), Q(L"!≈", NRE),
		Q(L"<", LT), Q(L"<=", LE), Q(L"≤", LE),
		Q(L">", GT), Q(L">=", GE), Q(L"≥", GE),
		Q(L"+", PLUS), Q(L"-", MINUS), Q(L"*", MUL), Q(L"/", DIV),
		Q(L"//", REMDIV), Q(L"%", MOD), Q(L"^", POW), Q(L"@", MATMUL),
		Q(L"!", EXCL), Q(L"||", OR), Q(L"&", AND), Q(L"<>", XOR),
		Q(L"+=", IADD), Q(L"-=", ISUB), Q(L"*=", IMUL), Q(L"/=", IDIV),
		Q(L"//=", IREMDIV), Q(L"%=", IMOD), Q(L"^=", IPOW), Q(L"@=", IMATMUL),
		Q(L"||=", IOR), Q(L"&=", IAND), Q(L"<>=", IXOR),
		Q(L"+-", PM), Q(L"±", PM), Q(L"²", SQR), Q(L"³", CUBE), Q(L"√", SQRT),
		Q(L"(", PAREN_L), Q(L")", PAREN_R),
		Q(L"[", SBRACK_L), Q(L"]", SBRACK_R),
		Q(L"{", CBRACK_L), Q(L"}", CBRACK_R),
		Q(L".", DOT), Q(L",", COMMA), Q(L":", COLON), Q(L";", SEMICOLON),
		Q(L"=", SET), Q(L"?", TERNARY), Q(L"->", ARROW), Q(L"|", ABS_BRACKET),
		Q(L"#", LENGTH), Q(L"'", FUNC_DERIV), Q(L"∂", DERIVATIVE),
		Q(L"∈", KW_IN), Q(L"!∈", KW_NOT_IN), Q(L"∉", KW_NOT_IN),
		Q(L"⊂", SUBSET), Q(L"⊃", SUPERSET);
	};
}

__attribute__((destructor)) void teardown_token_luts() {
	free_DictTree(keyword_tokens);
	free_HashMap(symbol_tokens);
}

#undef P
#undef Q


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
	lexer->pos=0;
	lexer->buffsize=0;

	lexer->line=1;
	lexer->col=1;
	return lexer;
}

void free_Lexer(Lexer *lexer) {
	if (lexer==NULL)
		return;
	
	free(lexer);
}

static bool next_chunk(Lexer *lexer) {
	wchar_t *buffer=lexer->buffer;
	ushort buffsize=lexer->buffsize;

	if (buffsize==0) {
		if (fgetws(buffer, LEXER_BUFFSIZE, lexer->file)==NULL)
			return false;

		lexer->buffsize=wcsnlen(buffer, LEXER_BUFFSIZE);
		lexer->pos=0;
		return true;
	}

	lexer->col+=buffsize-1;
	lexer->pos=1;
	buffer[0]=buffer[buffsize-1];
	if (buffer[0]==L'\n') {
		lexer->line++;
		lexer->col=0;
	}

	if (fgetws(buffer+1, LEXER_BUFFSIZE-1, lexer->file)==NULL) {
		lexer->buffsize=1;
		return false;
	}

	lexer->buffsize=wcsnlen(buffer+1, LEXER_BUFFSIZE-1)+1;
	return true;
}

static wchar_t curr_char(Lexer *lexer) {
	if (lexer->pos<0) {
		perror("Cannot get current char");
		return L'\0';
	}

	return lexer->buffer[lexer->pos];
}

static wchar_t next_char(Lexer *lexer) {
	short pos=lexer->pos+1;
	if (pos<0) {
		perror("Cannot get next char: pos < -1");
		return L'\0';
	}

	if (pos<lexer->buffsize) {
		lexer->pos=pos;
		return lexer->buffer[pos];
	}

	if (!next_chunk(lexer))
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


Token make_Token(TokenType type, const wchar_t *value) {
	Token token = (Token){type, .value=NULL};
	size_t value_size = 0;
	if (type==INDENT)
		value_size=(size_t)value;
	else if (value!=NULL)
		token.value=wcsdup_len(value, &value_size);

	token.value_size=value_size;
	return token;
}

Token make_Token_error(Lexer *lexer, const wchar_t *value) {
	Token token = (Token){TT_ERROR, .value=NULL};
	size_t value_size = 0;
	if (value!=NULL)
		token.value=wcsdup_len(value, &value_size);

	token.value_size=value_size;
	token.line=lexer->line;
	token.col=lexer->col+lexer->pos;
	return token;
}

#define MAKE_TOKEN(toktype, val) make_Token(toktype, val)

static Token handle_string(Lexer *lexer) {
	ushort i, buffsize=32;
	wchar_t *buffer=calloc(buffsize, sizeof(wchar_t));

	wchar_t c=next_char(lexer);
	for (i=0; c!=L'"'; c=next_char(lexer)) {
		if (c==L'\0') {
			free(buffer);
			return make_Token_error(lexer, L"Unexpected EOF");
		} //TODO: handle backslash
		
		if (i>=(buffsize-1)) {
			buffsize*=BUFF_GROWRATE;
			buffer=realloc(buffer, buffsize*sizeof(wchar_t));
		}
		buffer[i++]=c;
	}

	buffer[i++]=L'\0';
	return (Token){STRING, .value=buffer, .value_size=i};
}

static Token handle_dollar(Lexer *lexer) {
	Token token;
	wchar_t c, str[2];

	c=next_char(lexer);
	switch (c) {
		case L'"':
			token=handle_string(lexer); //TODO: fstring parsing
			if (token.type!=TT_ERROR)
				token.type=FSTRING;
			return token;
		case L'\'':
			c=next_char(lexer);
			if (c==L'\\') { //TODO: handle hex
				c=next_char(lexer);
				c=(c<ESCAPE_CODES_SIZE)? escape_codes[c]: L'\0';
				if (c==L'\0')
					return make_Token_error(lexer, L"Unknown escape sequence");
			} else if (c==L'\'')
				return make_Token_error(lexer, L"Empty character literal");

			if (next_char(lexer)!=L'\'')
				return make_Token_error(lexer, L"Unclosed character literal");
			
			str[0]=c; str[1]=L'\0';
			return MAKE_TOKEN(LCHAR, str);
	}

	prev_char(lexer);
	return MAKE_TOKEN(DOLLAR, NULL);
}

static Token skip_comment(Lexer *lexer) {
	do {
		if (!next_chunk(lexer))
			return MAKE_TOKEN(TT_EOF, NULL);
	} while (lexer->buffer[0]!='\n');

	return MAKE_TOKEN(NEWLINE, NULL);
}

static Token handle_backslash(Lexer *lexer) {
	wchar_t c=next_char(lexer);
	if (c==L'\\')
		return skip_comment(lexer);

	if (c==L'\n')
		return MAKE_TOKEN(INDENT, (void *)1);

	return make_Token_error(lexer, L"Expected newline after "
							L"line continuation character, got ...");
}

static Token handle_whitespace(Lexer *lexer) {
	ushort count=0;
	wchar_t c=curr_char(lexer);
	if (c==L'\n')
		return MAKE_TOKEN(NEWLINE, NULL);
	
	for (; c==L' ' or c==L'\t'; c=next_char(lexer))
		count+=(c==L'\t')? TABSIZE: 1;
	
	prev_char(lexer);
	return MAKE_TOKEN(INDENT, (void *)(count/TABSIZE));
}

static Token handle_number(Lexer *lexer) {
	ushort i=0, buffsize=16;
	wchar_t *buffer=calloc(buffsize, sizeof(wchar_t));
	buffer[0]=curr_char(lexer);
	
	wchar_t c=next_char(lexer);
	bool is_real=true, is_float=false;
	for (; c!=L'\0'; c=next_char(lexer)) {
		if (c==L'.') {
			if (is_float)
				return make_Token_error(lexer, L"Unexpected second dot");
			is_float=true;
		} else if (c==imagchar) {
			is_real=false;
			goto end;
		} else if (!iswdigit(c)) {
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
	Token token = MAKE_TOKEN(number_types[2*is_real+is_float], buffer);
	free(buffer);
	return token;
}

static Token handle_identifier(Lexer *lexer) {
	ushort i=0, buffsize=16;
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
		return MAKE_TOKEN(tt, NULL);
	}

	Token token = MAKE_TOKEN(IDENTIFIER, buffer);
	free(buffer);
	return token;
}

#define SEND_TOKEN(tok) { \
	Token tok1=tok; \
	tok1.line=line; tok1.col=col; \
	return tok1; \
}

Token next_token(Lexer *lexer) {
	wchar_t c=next_char(lexer);
	ushort line=lexer->line, col=lexer->col+lexer->pos;

	if (c==L'\0')
		return MAKE_TOKEN(TT_EOF, NULL);
	
	if (valid_whitespace(c)) SEND_TOKEN(handle_whitespace(lexer))
	if (iswdigit(c)) SEND_TOKEN(handle_number(lexer))
	if (valid_alpha(c)) SEND_TOKEN(handle_identifier(lexer))

	// Handle simple tokens
	TokenType tt0, tt = (c<SINGLE_TOKEN_SIZE)? single_tokens[c]: TT_EOF;
	if (tt!=TT_EOF) SEND_TOKEN(((Token){tt, .value=NULL}))

	// Handle compound tokens:
	wchar_t buffer[8]={c, L'\0'};
	tt=(TokenType)HashMap_get(symbol_tokens, buffer);
	if (tt!=TT_EOF) {
		for (ushort i=1; i<7 && tt!=TT_EOF; i++) {
			tt0=tt; c=next_char(lexer);
			buffer[i]=c; buffer[i+1]=L'\0';
			tt=(TokenType)HashMap_get(symbol_tokens, buffer);
		}

		prev_char(lexer);
		SEND_TOKEN(((Token){tt0, .value=NULL}))
	}

	// Handle special symbols
	switch (c) { //TODO: handle_dot: ..., .05
		case L'"': SEND_TOKEN(handle_string(lexer))
		case L'$': SEND_TOKEN(handle_dollar(lexer))
		case L'\\': SEND_TOKEN(handle_backslash(lexer))
	}

	return make_Token_error(lexer, L"Unknown char"); //TODO: handle char format
}

#undef SEND_TOKEN
#undef MAKE_TOKEN
