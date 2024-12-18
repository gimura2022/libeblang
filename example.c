#include <stdio.h>
#include <stdlib.h>

#include <eblang/eblang.h>
#include <eblang/parse.h>

#include <gstd/dynarr.h>
#include <gstd/allocators.h>

int main(int argc, char* argv[])
{
	struct gstd__memmanager memmanager = {
		.allocator   = malloc,
		.deallocator = free,
	};

	eblang__init(&memmanager);

	struct eblang_parse__parser parser = {0};
	eblang_parse__init_default_parser(&parser);

	struct eblang_parse__tok_list out_toks = {0};
	eblang_parse__parse(&parser, &out_toks, "},5$*=");

	for (int i = 0; i < gstd__dynarr_len(&out_toks.toks); i++) {
		const struct eblang_parse__kw* kw = gstd__dynarr_get(&out_toks.toks, i);

		printf("Keyword '%c':\n", kw->sym);
	}

	eblang_parse__free_tok_list(&out_toks);
	eblang_parse__free_parser(&parser);

	return 0;
}
