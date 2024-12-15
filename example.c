#include <stdio.h>
#include <stdlib.h>

#include <eblang/eblang.h>
#include <eblang/parse.h>
#include <gstd/dunarr.h>
#include <gstd/allocators.h>

int main(int argc, char* argv[])
{
	struct gstd__memmanager memmanager = {
		.allocator   = malloc,
		.deallocator = free,
	};

	eblang__init(&memmanager);

	struct eblang__command* out;
	size_t out_size;
	eblang__parse(&out, &out_size, "},5$f0o0\"fuck\"$f0o4,0^f0o0");

	for (int i = 0; i < out_size; i++) {
		struct eblang__command* cmd = &out[i];
		printf("found command %c with args:\n", cmd->type);

		for (int j = 0; j < cmd->args_count; j++) {
			struct eblang__arg* arg = &cmd->args[j];
			printf("	%i: type: %c\n", j, arg->type);
		}
	}

	eblang__free_parsed(out, out_size);

	return 0;
}
