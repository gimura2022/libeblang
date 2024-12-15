#ifndef _eblang_ebal_h
#define _eblang_ebal_h

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "parse.h"

enum {
	EBLANGCMD__ALLOC_VAL = 0,
	EBLANGCMD__ALLOC_VAR,
};

struct eblang_ebal__command {
	int type;
	
	union {
		struct {
			uint8_t val;
		} alloc_val;
		struct {
			uint8_t var;
			uint8_t off;
		} alloc_var;
	} args;
};

struct eblang_ebal__code_block {
	size_t command_count;
	struct eblang_ebal__command* commands;
};

struct eblang_ebal {
	size_t code_block_count;
	struct eblang_ebal__code_block* code_blocks;

	struct eblang_ebal__code_block* crnt_code_block;
};

bool eblang_ebal__create(struct eblang_ebal* ebal, const struct eblang_parse__command* commands,
		size_t command_count);

#endif
