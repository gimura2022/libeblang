#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <gstd/dynarr.h>
#include <gstd/dynarr_malloc.h>
#include <gstd/utils.h>

#include "parse.h"
#include "eblang.h"

void eblang_parse__init_default_parser(struct eblang_parse__parser* parser)
{
	parser->ignore_chars = "\t\n\r ";
	
	gstd__dynarr_malloc_create(&parser->keywords, eblang__memmanager,
			sizeof(struct eblang_parse__kw_def));
	gstd__dynarr_malloc_create(&parser->args, eblang__memmanager, sizeof(struct eblang_parse__arg_def));

#	define add_kw(c) \
		gstd__dynarr_push_end(&parser->keywords, &(struct eblang_parse__kw_def) { .sym = c })

	add_kw(EBLANGKW__ADD);
	add_kw(EBLANGKW__CALL);
	add_kw(EBLANGKW__ALLOC);
	add_kw(EBLANGKW__DIV);
	add_kw(EBLANGKW__DEALLOC);
	add_kw(EBLANGKW__EXIT);
	add_kw(EBLANGKW__GET_ADDR);
	add_kw(EBLANGKW__JMP);
	add_kw(EBLANGKW__JMPIF);
	add_kw(EBLANGKW__LABEL);
	add_kw(EBLANGKW__MUL);
	add_kw(EBLANGKW__SET);
	add_kw(EBLANGKW__SUB);
	add_kw(EBLANGKW__VAR_FROM_ADDR);

#	undef add_kw

#	define add_arg(c, fn) \
		gstd__dynarr_push_end(&parser->args, \
				&(struct eblang_parse__arg_def) { .sym = c, .parser = fn })

	add_arg(EBLANGARG__AND    , NULL);
	add_arg(EBLANGARG__OR     , NULL);
	add_arg(EBLANGARG__XOR    , NULL);
	add_arg(EBLANGARG__EQUALS , NULL);
	add_arg(EBLANGARG__EXPORT , NULL);
	add_arg(EBLANGARG__LEFT   , NULL);
	add_arg(EBLANGARG__RIGHT  , NULL);
	add_arg(EBLANGARG__START  , NULL);

	add_arg(EBLANGARG__GENERAL_CHAR, eblang_parse__parse_char);

	add_arg(EBLANGARG__GENERAL_NUM , eblang_parse__parse_num);
	add_arg(EBLANGARG__OFFSET      , eblang_parse__parse_num);
	add_arg(EBLANGARG__VARIABLE    , eblang_parse__parse_num);
	add_arg(EBLANGARG__INSTRUCTION , eblang_parse__parse_num);

	add_arg(EBLANGARG__STR, eblang_parse__parse_str);

#	undef add_arg
}

void eblang_parse__free_parser(struct eblang_parse__parser* parser);

static bool is_ignore_char(const struct eblang_parse__parser* parser, char c);
static int parse_kw(struct eblang_parse__parser* parser, struct eblang_parse__kw* kw, const char* str);

bool eblang_parse__parse(struct eblang_parse__parser* parser,
		struct eblang_parse__tok_list* out, const char* str)
{
	gstd__dynarr_malloc_create(&out->toks, eblang__memmanager, sizeof(struct eblang_parse__kw));

	for (const char* c = str; c - str < strlen(str);) {
		if (is_ignore_char(parser, *c)) {
			c++;
			continue;
		}

		struct eblang_parse__kw kw = {0};
		int shift                  = parse_kw(parser, &kw, c);

		if (shift == -1)
			return false;

		c += shift;

		gstd__dynarr_push_end(&out->toks, &kw);
	}

	return true;
}

void eblang_parse__free_tok_list(struct eblang_parse__tok_list* list)
{
	for (int i = 0; i < gstd__dynarr_len(&list->toks); i++) {
		struct eblang_parse__kw* kw = gstd__dynarr_get(&list->toks, i);

		for (int j = 0; j < gstd__dynarr_len(&kw->args); j++) {
			struct eblang_parse__arg* arg = gstd__dynarr_get(&kw->args, j);

			if (arg->data != NULL)
				eblang__memmanager->deallocator(arg->data);
		}

		gstd__dynarr_free(&kw->args);
	}

	gstd__dynarr_free(&list->toks);
}

int eblang_parse__parse_char(void** data, const char* str, struct eblang_parse__parser* parser)
{
	*data            = eblang__memmanager->allocator(sizeof(char));
	*((char*) *data) = *str;

	return 1;
}

int eblang_parse__parse_num(void** data, const char* str, struct eblang_parse__parser* parser)
{
	*data = eblang__memmanager->allocator(sizeof(uint8_t));

	const char* c;
	for (c = str; c - str < strlen(str);) {
		if (!isdigit(*c))
			return -1;

		uint8_t val = *c - '0';
		c++;

		switch (*c) {
		case EBLANGNUM__10_13:
			if (val > 3)
				return -1;

			val += 10;
			break;

		case EBLANGNUM__14_19:
			if (val < 4)
				return -1;

			val += 10;
			break;

		case EBLANGNUM__20_90:
			if (val < 2)
				return -1;

			val *= 10;
			break;

		case EBLANGNUM__100_900:
			val *= 100;
			break;
		}
		c++;
	}

	return c - str;
}

int eblang_parse__parse_str(void** data, const char* str, struct eblang_parse__parser* parser)
{
	const char* c;
	size_t str_len;

	for (c = str, str_len = 0; *c != EBLANGARG__STR && *c != '\0'; c++)
		str_len++;

	*data = eblang__memmanager->allocator(str_len);

	for (c = str; *c != EBLANGARG__STR && *c != '\0'; c++)
		((char*) *data)[c - str] = *c;

	return c - str;
}

static bool is_ignore_char(const struct eblang_parse__parser* parser, char c)
{
	return strchr(parser->ignore_chars, c);
}

static const struct eblang_parse__kw_def* get_kw_def(const struct eblang_parse__parser* parser, char c);
static int parse_arg(struct eblang_parse__parser* parser, struct eblang_parse__arg* arg, const char* str);

static int parse_kw(struct eblang_parse__parser* parser, struct eblang_parse__kw* kw, const char* str)
{
	const char* c = str;

	const struct eblang_parse__kw_def* def = get_kw_def(parser, *c);
	if (def == NULL)
		return -1;

	kw->sym = def->sym;

	gstd__dynarr_malloc_create(&kw->args, eblang__memmanager, sizeof(struct eblang_parse__arg_def));

	for (c++; c - str < strlen(c);) {
		if (is_ignore_char(parser, *c)) {
			c++;
			continue;
		}

		struct eblang_parse__arg arg = {0};
		int shift                    = parse_arg(parser, &arg, c);

		if (shift == -1)
			return -1;

		c += shift;

		gstd__dynarr_push_end(&kw->args, &arg);
	}

	return c - str;
}

static const struct eblang_parse__kw_def* get_kw_def(const struct eblang_parse__parser* parser, char c)
{
	for (int i = 0; i < gstd__dynarr_len(&parser->keywords); i++) {
		const struct eblang_parse__kw_def* def = gstd__dynarr_get(&parser->keywords, i);

		if (def->sym == c)
			return def;
	}

	return NULL;
}

static const struct eblang_parse__arg_def* get_arg_def(const struct eblang_parse__parser* parser, char c);

static int parse_arg(struct eblang_parse__parser* parser, struct eblang_parse__arg* arg, const char* str)
{
	const char* c = str;

	const struct eblang_parse__arg_def* def = get_arg_def(parser, *c);
	if (def == NULL)
		return -1;

	arg->sym  = def->sym;
	arg->data = NULL;

	if (def->parser == NULL)
		return 1;

	c++;

	int shift = def->parser(&arg->data, c, parser);
	if (shift == -1)
		return -1;

	return shift + 1;
}

static const struct eblang_parse__arg_def* get_arg_def(const struct eblang_parse__parser* parser, char c)
{
	for (int i = 0; i < gstd__dynarr_len(&parser->args); i++) {
		const struct eblang_parse__arg_def* def = gstd__dynarr_get(&parser->args, i);

		if (def->sym == c)
			return def;
	}

	return NULL;
}
