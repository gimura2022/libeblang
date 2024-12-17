#ifndef _eblang_parse_h
#define _eblang_parse_h

#include <stddef.h>
#include <stdbool.h>

#include <gstd/dynarr.h>

enum {
	EBLANGKW__ALLOC         = '}',
	EBLANGKW__VAR_FROM_ADDR = '{',
	EBLANGKW__DEALLOC       = '#',

	EBLANGKW__SET = '$',
	EBLANGKW__ADD = '?',
	EBLANGKW__SUB = '%',
	EBLANGKW__MUL = 'x',
	EBLANGKW__DIV = '.',

	EBLANGKW__JMP   = ']',
	EBLANGKW__JMPIF = '[',

	EBLANGKW__GET_ADDR = '~',
	EBLANGKW__CALL     = '/',

	EBLANGKW__LABEL = '*',
	EBLANGKW__EXIT  = '=',
};

enum {
	EBLANGARG__STR          = '"',
	EBLANGARG__GENERAL_CHAR = ':',

	EBLANGARG__GENERAL_NUM = ',',
	EBLANGARG__VARIABLE    = 'f',
	EBLANGARG__OFFSET      = 'o',
	EBLANGARG__INSTRUCTION = 'p',

	EBLANGARG__EQUALS = 'e',
	EBLANGARG__OR     = 'r',
	EBLANGARG__XOR    = 'x',
	EBLANGARG__AND    = 'a',
	EBLANGARG__RIGHT  = 't',
	EBLANGARG__LEFT   = 'l',

	EBLANGARG__START  = 'S',
	EBLANGARG__EXPORT = 'E',
};

enum {
	EBLANGNUM__10_13   = '+',
	EBLANGNUM__14_19   = '>',
	EBLANGNUM__20_90   = ';',
	EBLANGNUM__100_900 = '&',
};

struct eblang_parse__kw_def {
	char sym;
};

struct eblang_parse__parser {
	struct gstd__dynarr keywords;
	struct gstd__dynarr args;

	const char* ignore_chars;
};

typedef int (*eblang_parse__arg_parser_f)(void**, const char*, struct eblang_parse__parser*);

struct eblang_parse__arg_def {
	char sym;
	eblang_parse__arg_parser_f parser;
};

struct eblang_parse__kw {
	char sym;
	struct gstd__dynarr args;
};

struct eblang_parse__arg {
	char sym;
	void* data;
};

struct eblang_parse__tok_list {
	struct gstd__dynarr toks;
};

void eblang_parse__init_default_parser(struct eblang_parse__parser* parser);
void eblang_parse__free_parser(struct eblang_parse__parser* parser);

bool eblang_parse__parse(struct eblang_parse__parser* parser,
		struct eblang_parse__tok_list* out, const char* str);
void eblang_parse__free_tok_list(struct eblang_parse__tok_list* list);

int eblang_parse__parse_char(void** data, const char* str, struct eblang_parse__parser* parser);
int eblang_parse__parse_num(void** data, const char* str, struct eblang_parse__parser* parser);
int eblang_parse__parse_str(void** data, const char* str, struct eblang_parse__parser* parser);

#endif
