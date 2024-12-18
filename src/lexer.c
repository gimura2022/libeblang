#include <stddef.h>

#include <gstd/dynarr.h>
#include <gstd/dynarr_malloc.h>
#include <gstd/utils.h>

#include "lexer.h"
#include "eblang.h"
#include "parse.h"

static bool parse_command(const struct eblang_lexer__lexer* lexer, struct eblang_lexer__command* command,
		const struct eblang_parse__kw* kw, struct eblang_lexer__lexed* out);

bool eblang_lexer__parse(const struct eblang_lexer__lexer* lexer, struct eblang_lexer__lexed* out,
		const struct eblang_parse__tok_list* toks)
{
	gstd__dynarr_malloc_create(&out->code_blocks, eblang__memmanager,
			sizeof(struct eblang_lexer__code_block));

	struct eblang_lexer__code_block first_code_block = {
		.type     = EBLANGCODEBLOCKTYPE__START,
		.commands = {0},
	};

	gstd__dynarr_malloc_create(&first_code_block.commands, eblang__memmanager,
			sizeof(struct eblang_lexer__command));

	gstd__dynarr_push_end(&out->code_blocks, &first_code_block);

	for (int i = 0; i < gstd__dynarr_len(&toks->toks); i++) {
		const struct eblang_parse__kw* kw    = gstd__dynarr_get(&toks->toks, i);
		struct eblang_lexer__command command = {0};
		
		continue_or_retrun(parse_command(lexer, &command, kw, out));

		struct eblang_lexer__code_block* crnt_code_block = gstd__dynarr_get(&out->code_blocks,
				gstd__dynarr_len(&out->code_blocks) - 1);

		gstd__dynarr_push_end(&crnt_code_block->commands, &command);
	}

	return true;
}

void eblang_lexer__free_parsed(struct eblang_lexer__lexed* out)
{
}

static const struct eblang_lexer__command_def* get_command_def(const struct eblang_lexer__lexer* lexer,
		const struct eblang_parse__kw* kw);

static bool parse_command(const struct eblang_lexer__lexer* lexer, struct eblang_lexer__command* command,
		const struct eblang_parse__kw* kw, struct eblang_lexer__lexed* out)
{
	const struct eblang_lexer__command_def* def = get_command_def(lexer, kw);
	if (def == NULL)
		return false;

	command->type = def->type;

	if (def->parser == NULL)
		return true;

	continue_or_retrun(def->parser(lexer, out, &command->command_data));

	return true;
}

static const struct eblang_lexer__command_def* get_command_def(const struct eblang_lexer__lexer* lexer,
		const struct eblang_parse__kw* kw)
{
	for (int i = 0; i < gstd__dynarr_len(&lexer->commands); i++) {
		const struct eblang_lexer__command_def* def = gstd__dynarr_get(&lexer->commands, i);

		for (int j = 0; j < gstd__dynarr_len(&kw->args) && j < def->arg_count; j++) {
			const struct eblang_parse__arg* arg         = gstd__dynarr_get(&kw->args, j);
			const struct eblang_parse__arg_def* arg_def = &def->args[j];

			if (arg->sym != arg_def->sym)
				goto for_continue;
		}

		if (def->kw.sym != kw->sym)
			goto for_continue;

		return def;

for_continue:;
	}

	return NULL;
}
