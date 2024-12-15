#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>

#include "parse.h"
#include "eblang.h"

static int parse_command(struct eblang__command* command, const char* str);

bool eblang__parse(struct eblang__command** out, size_t* out_size, const char* str)
{
	struct eblang__command cmd;
	int shift, i;
	const char* new_str = str;

	*out_size = 0;
	*out      = NULL;

	for (shift = parse_command(&cmd, new_str), i = 0, new_str = str; shift > 0; new_str += shift,
			shift = parse_command(&cmd, new_str), i++) {
		if (shift == -1)
			return false;

		(*out_size)++;
		struct eblang__command* new_out =
			eblang__memmanager->allocator(sizeof(struct eblang__command) * (*out_size));

		if (*out != NULL) {
			memcpy(new_out, (*out), sizeof(struct eblang__command) * ((*out_size) - 1));
			eblang__memmanager->deallocator(*out);
		}

		*out = new_out;

		(*out)[i] = cmd;
	}

	return true;
}

void eblang__free_parsed(struct eblang__command* commands, size_t size)
{
	for (int i = 0; i < size; i++) {
		struct eblang__command* cmd = &commands[i];
		
		for (int j = 0; j < cmd->args_count; j++) {
			struct eblang__arg* arg = &cmd->args[j];

			if (arg->data != NULL)
				eblang__memmanager->deallocator(arg->data);
		}

		eblang__memmanager->deallocator(cmd->args);
	}

	eblang__memmanager->deallocator(commands);
}

static bool is_command(char c);
static int parse_arg(struct eblang__arg* arg, const char* str);

static int parse_command(struct eblang__command* command, const char* str)
{
	if (*str == '\0')
		return 0;

	const char* c = str;
	if (!is_command(*c))
		return -1;

	command->type       = *c;
	command->args_count = 0;
	command->args       = NULL;

	c++;

	int shift, i;
	struct eblang__arg arg;

	for (shift = parse_arg(&arg, c), i = 0; shift > 0 && *c != '\0'; i++,
			c += shift, shift = parse_arg(&arg, c)) {
		if (shift == -1)
			return -1;

		command->args_count++;
		struct eblang__arg* new_args =
			eblang__memmanager->allocator(sizeof(struct eblang__arg) * command->args_count);

		if (command->args != NULL) {
			memcpy(new_args, command->args,
					sizeof(struct eblang__arg) * (command->args_count - 1));
			eblang__memmanager->deallocator(command->args);
		}

		command->args = new_args;

		command->args[i] = arg;
	}

	return c - str;
}

static bool is_command(char c)
{
	return c == EBLANGKW__ADD ||
		c == EBLANGKW__ALLOC ||
		c == EBLANGKW__GET ||
		c == EBLANGKW__GET_ADDR ||
		c == EBLANGKW__VAR_FROM_ADDR ||
		c == EBLANGKW__JMP ||
		c == EBLANGKW__JMPIF ||
		c == EBLANGKW__PRINT ||
		c == EBLANGKW__SET ||
		c == EBLANGKW__SUB ||
		c == EBLANGKW__DEALLOC;
}

static int parse_arg_num(void** data, const char* str);
static int parse_arg_str(void** data, const char* str);

static int parse_arg(struct eblang__arg* arg, const char* str)
{
	switch (*str) {
	case '\0':
		return 0;
	
	case EBLANGARG__AND:
	case EBLANGARG__EQUALS:
	case EBLANGARG__LEFT:
	case EBLANGARG__RIGHT:
	case EBLANGARG__OR:
	case EBLANGARG__XOR:
		arg->type = *str;
		arg->data = NULL;

		return 1;

	case EBLANGARG__GENERAL_NUM:
	case EBLANGARG__VARIABLE:
	case EBLANGARG__OFFSET:
	case EBLANGARG__INSTRUCTION:
		arg->type = *str;
		return parse_arg_num(&arg->data, str);

	case EBLANGARG__GENERAL_CHAR:
		arg->type = *str;
		arg->data = eblang__memmanager->allocator(sizeof(char));
		*((char*) arg->data) = *str;

		return 2;

	case EBLANGARG__STR:
		arg->type = *str;
		return parse_arg_str(&arg->data, str);

	default:
		return -1;
	}
}

static bool is_num_directive(char c);

static int parse_arg_num(void** data, const char* str)
{
	*data = eblang__memmanager->allocator(sizeof(uint8_t));
	*((uint8_t*) *data) = 0;

	const char* c = str + 1;
	while (true) {
		if (!(isdigit(*c) && *c != '\0'))
			break;

		uint8_t num = *c - '0';

		c++;
		if (!(is_num_directive(*c) && *c != '\0')) {
			*((uint8_t*) *data) += num;
			break;
		}

		switch (*c) {
		case EBLANGNUM__100_900:
			*((uint8_t*) *data) += num * 100;
			break;

		case EBLANGNUM__20_90:
			if (*((uint8_t*) *data) < 2)
				return -1;

			*((uint8_t*) *data) += num * 10;
			break;

		case EBLANGNUM__10_13:
			if (*((uint8_t*) *data) > 3)
				return -1;

			*((uint8_t*) *data) += 10;
			break;

		case EBLANGNUM__14_19:
			if (*((uint8_t*) *data) < 4)
				return -1;

			*((uint8_t*) *data) += 10;
			break;
		}
		c++;
	}

	return c - str;
}

static int parse_arg_str(void** data, const char* str)
{
	const char* c = str;
	c++;

	for (; *c != '\0' && *c != EBLANGARG__STR; c++);

	size_t len = c - str;
	*data = eblang__memmanager->allocator(len);

	for (c = str; *c != '\0' && *c != EBLANGARG__STR; c++) {
		((char*) (*data))[c - str] = *c;
	}

	return len + 1;
}

static bool is_num_directive(char c)
{
	return c == EBLANGNUM__10_13 ||
		c == EBLANGNUM__14_19 ||
		c == EBLANGNUM__20_90 ||
		c == EBLANGNUM__100_900;
}
