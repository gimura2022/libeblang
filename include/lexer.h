#ifndef _eblang_lexer_h
#define _eblang_lexer_h

#include <stdbool.h>

#include <gstd/dynarr.h>

#include "parse.h"

#define MAX_ARGS 32

struct eblang_lexer__lexer {
	struct gstd__dynarr commands;
};

typedef void* eblang_lexer__command_data_t;

struct eblang_lexer__command {
	int type;
	eblang_lexer__command_data_t command_data;
};

enum {
	EBLANGCODEBLOCKTYPE__UNNAMED = 0,
	EBLANGCODEBLOCKTYPE__NAMED,
	EBLANGCODEBLOCKTYPE__EXPORT,
	EBLANGCODEBLOCKTYPE__START,
};

struct eblang_lexer__code_block {
	int type;
	struct gstd__dynarr commands;
};

struct eblang_lexer__lexed {
	struct gstd__dynarr code_blocks;
};

typedef bool (*eblang_lexer__command_parser_f)(const struct eblang_lexer__lexer*,
		struct eblang_lexer__lexed*, eblang_lexer__command_data_t*);

struct eblang_lexer__command_def {
	int type;

	struct eblang_parse__kw_def kw;
	struct eblang_parse__arg_def args[MAX_ARGS];
	size_t arg_count;

	eblang_lexer__command_parser_f parser;
};

bool eblang_lexer__parse(const struct eblang_lexer__lexer* lexer, struct eblang_lexer__lexed* out,
		const struct eblang_parse__tok_list* toks);
void eblang_lexer__free_parsed(struct eblang_lexer__lexed* out);

#endif
