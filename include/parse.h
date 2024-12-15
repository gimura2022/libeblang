#ifndef _eblang_parse_h
#define _eblang_parse_h

#include <stddef.h>
#include <stdbool.h>

enum {
	EBLANGKW__ALLOC         = '}',
	EBLANGKW__VAR_FROM_ADDR = '{',
	EBLANGKW__SET           = '$',
	EBLANGKW__ADD           = '?',
	EBLANGKW__SUB           = '%',
	EBLANGKW__PRINT         = '^',
	EBLANGKW__GET           = '!',
	EBLANGKW__JMP           = ']',
	EBLANGKW__JMPIF         = '[',
	EBLANGKW__GET_ADDR      = '~',
	EBLANGKW__DEALLOC       = '#',
};

enum {
	EBLANGARG__STR          = '"',
	EBLANGARG__GENERAL_CHAR = ':',
	EBLANGARG__GENERAL_NUM  = ',',
	EBLANGARG__VARIABLE     = 'f',
	EBLANGARG__OFFSET       = 'o',
	EBLANGARG__INSTRUCTION  = 'p',
	EBLANGARG__EQUALS       = 'e',
	EBLANGARG__OR           = 'r',
	EBLANGARG__XOR          = 'x',
	EBLANGARG__AND          = 'a',
	EBLANGARG__RIGHT        = 't',
	EBLANGARG__LEFT         = 'l',
};

enum {
	EBLANGNUM__10_13   = '+',
	EBLANGNUM__14_19   = '>',
	EBLANGNUM__20_90   = ';',
	EBLANGNUM__100_900 = '&',
};

struct eblang__arg {
	int type;
	void* data;
};

struct eblang__command {
	int type;

	struct eblang__arg* args;
	size_t args_count;
};

bool eblang__parse(struct eblang__command** out, size_t* out_size, const char* str);
void eblang__free_parsed(struct eblang__command* commands, size_t size);

#endif
