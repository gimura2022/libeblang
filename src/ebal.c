#include <stdint.h>
#include <string.h>

#include "eblang.h"
#include "parse.h"
#include "ebal.h"

#define continue_or_return(x) ({ if (!x) return false; })
#define continue_or_return_break(x) ({ if (!x) return false; break; })

static bool parse_command(const struct eblang_parse__command* cmd, struct eblang_ebal* ebal);

static struct eblang_ebal__code_block* add_code_block(struct eblang_ebal* ebal);

bool eblang_ebal__create(struct eblang_ebal* ebal, const struct eblang_parse__command* commands,
		size_t command_count)
{
	ebal->code_blocks      = NULL;
	ebal->code_block_count = 0;
	ebal->crnt_code_block  = NULL;

	struct eblang_ebal__code_block* first_code_block = add_code_block(ebal);
	first_code_block->command_count = 0;
	first_code_block->commands      = NULL;

	for (int i = 0; i < command_count; i++) {
		const struct eblang_parse__command* cmd = &commands[i];
		continue_or_return(parse_command(cmd, ebal));
	}

	return true;
}

static struct eblang_ebal__command* add_command(struct eblang_ebal__code_block* code_block);

static bool parse_alloc(const struct eblang_parse__command* cmd, struct eblang_ebal* ebal);

static bool parse_command(const struct eblang_parse__command* cmd, struct eblang_ebal* ebal)
{
	switch (cmd->type) {
	case EBLANGKW__ALLOC: continue_or_return_break(parse_alloc(cmd, ebal));

	default:
		return false;
	}

	return true;
}

static struct eblang_ebal__code_block* add_code_block(struct eblang_ebal* ebal)
{
	ebal->code_block_count++;

	struct eblang_ebal__code_block* new_code_blocks = eblang__memmanager->allocator(
			sizeof(struct eblang_ebal__code_block) * ebal->code_block_count);

	if (ebal->code_blocks != NULL) {
		memcpy(new_code_blocks, ebal->code_blocks, sizeof(struct eblang_ebal__code_block) *
				(ebal->code_block_count - 1));
		eblang__memmanager->deallocator(ebal->code_blocks);
	}

	ebal->code_blocks     = new_code_blocks;
	ebal->crnt_code_block = &ebal->code_blocks[ebal->code_block_count - 1];

	return ebal->crnt_code_block;
}

static struct eblang_ebal__command* add_command(struct eblang_ebal__code_block* code_block)
{
	code_block->command_count++;

	struct eblang_ebal__command* new_commands = eblang__memmanager->allocator(
			sizeof(struct eblang_ebal__command) * code_block->command_count);

	if (code_block->commands != NULL) {
		memcpy(new_commands, code_block->commands, sizeof(struct eblang_ebal__command) *
				(code_block->command_count - 1));
		eblang__memmanager->deallocator(code_block->commands);
	}

	code_block->commands = new_commands;

	return &code_block->commands[code_block->command_count - 1];
}

static bool parse_alloc_val(const struct eblang_parse__arg* val, struct eblang_ebal__command* command);
static bool parse_alloc_var(const struct eblang_parse__arg* var, const struct eblang_parse__arg* off,
		struct eblang_ebal__command* command);

static bool parse_alloc(const struct eblang_parse__command* cmd, struct eblang_ebal* ebal)
{
	struct eblang_ebal__command* command = add_command(ebal->crnt_code_block);

	switch (cmd->args_count) {
	case 1: continue_or_return_break(parse_alloc_val(&cmd->args[0], command));
	case 2: continue_or_return_break(parse_alloc_var(&cmd->args[0], &cmd->args[1], command));

	default:
		return false;
	}

	return true;
}

static bool parse_alloc_val(const struct eblang_parse__arg* val, struct eblang_ebal__command* command)
{
	if (val->type != EBLANGARG__GENERAL_NUM)
		return false;

	uint8_t data = *((uint8_t*) val->data);

	command->type               = EBLANGCMD__ALLOC_VAL;
	command->args.alloc_val.val = data;

	return true;
}

static bool parse_alloc_var(const struct eblang_parse__arg* var, const struct eblang_parse__arg* off,
		struct eblang_ebal__command* command)
{
	if (var->type != EBLANGARG__VARIABLE && off->type != EBLANGARG__OFFSET)
		return false;

	uint8_t var_id  = *((uint8_t*) var->data);
	uint8_t off_val = *((uint8_t*) off->data);

	command->type               = EBLANGCMD__ALLOC_VAR;
	command->args.alloc_var.off = off_val;
	command->args.alloc_var.var = var_id;

	return true;
}
