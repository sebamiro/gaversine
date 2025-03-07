#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "type.h"
#include "tokens.h"

#include "Arena.c"
#include "lex.c"

typedef enum type_value
{
	JSONValue_False,
	JSONValue_Null,
	JSONValue_True,
	JSONValue_Object,
	JSONValue_Array,
	JSONValue_Number,
	JSONValue_String,
} type_value;

typedef struct JSON_string
{
	u32		len;
	char*	str;
} JSON_string;

typedef enum type_number
{
	Number_int,
	Number_float,
} type_number;
typedef struct JSON_number
{
	type_number	typ;
	union
	{
		i64 num_int;
		f64 num_float;
	};
} JSON_number;

typedef struct json_value json_value;
typedef struct JSON_array
{
	u32			len;
	json_value* values;
} JSON_array;

typedef struct JSON_object
{
	u32				len;
	JSON_string*	keys;
	json_value*		values;
} JSON_object;

struct json_value
{
	type_value	typ;
	union
	{
		JSON_string	string;
		JSON_number	number;
		JSON_array	array;
		JSON_object	object;
	};
};

typedef struct Scanner
{
	u32		cur;
	tokens	tokens;
	char*	buf;
} Scanner;

void Scanner_expect_type(Scanner* scanner, type_token expected)
{
	assert(expected == scanner->tokens.types[scanner->cur]);
}

type_token Scanner_peek_type(Scanner* scanner)
{
	return scanner->tokens.types[scanner->cur];
}

u32 Scanner_string_len(Scanner* scanner)
{
	assert(scanner->tokens.types[scanner->cur] == Token_String);
	u32 len = scanner->tokens.starts[scanner->cur];

	while (scanner->buf[len] != 0 && scanner->buf[len] != '"')
	{
		++len;
	}
	assert(scanner->buf[len] != 0);
	return len - scanner->tokens.starts[scanner->cur];
}

u32 Scanner_object_len(Scanner* scanner)
{
	u32 i = scanner->cur;
	u32 len = 0;
	u32 in_object = 0;
	u32 in_array = 0;

	while (i < scanner->tokens.len)
	{
		switch (scanner->tokens.types[i])
		{
			case Token_BeginObject:
				++in_object;
				break;
			case Token_EndObject:
				if (in_object == 0 && in_array == 0)
				{
					return i != scanner->cur ? len + 1 : len;
				}
				--in_object;
				break;
			case Token_BeginArray:
				++in_array;
				break;
			case Token_EndArray:
				--in_array;
				break;
			case Token_ValueSeparator:
				if (in_object == 0 && in_array == 0)
				{
					++len;
				}
				break;
			default:
				break;
		}
		++i;
	}
	assert(0);
	return 0;
}

u32 Scanner_array_len(Scanner* scanner)
{
	u32 i = scanner->cur;
	u32 len = 0;
	u32 in_object = 0;
	u32 in_array = 0;

	while (i < scanner->tokens.len)
	{
		switch (scanner->tokens.types[i])
		{
			case Token_BeginObject:
				++in_object;
				break;
			case Token_EndObject:
				--in_object;
				break;
			case Token_BeginArray:
				++in_array;
				break;
			case Token_EndArray:
				if (in_object == 0 && in_array == 0)
				{
					return len != 0 ? len + 1 : len;
				}
				--in_array;
				break;
			case Token_ValueSeparator:
				if (in_object == 0 && in_array == 0)
				{
					++len;
				}
				break;
			default:
				break;
		}
		++i;
	}
	assert(0);
	return 0;
}

JSON_number parse_JSON_number(Scanner* scanner)
{
	Scanner_expect_type(scanner, Token_Number);
	JSON_number number;

	number.typ = Number_float;
	number.num_float = strtod(scanner->buf + scanner->tokens.starts[scanner->cur], NULL);
	scanner->cur++;
	return number;
}

JSON_string parse_JSON_string(Arena* arena, Scanner* scanner)
{
	JSON_string string;

	string.len = Scanner_string_len(scanner);
	string.str = (char*)Arena_alloc(arena, sizeof(char) * string.len);
	strncpy(string.str, scanner->buf + scanner->tokens.starts[scanner->cur], string.len);
	scanner->cur++;
	return string;
}

json_value parse_json_value(Arena* arena, Scanner* scanner);
JSON_array	parse_JSON_array(Arena* arena, Scanner* scanner)
{
	Scanner_expect_type(scanner, Token_BeginArray);
	scanner->cur++;
	JSON_array	array;
	array.len = Scanner_array_len(scanner);
	if (array.len > 0)
	{
		array.values = (json_value*)Arena_alloc(arena, sizeof(json_value) * array.len);
	}
	u32 current_element = 0;
	while (Scanner_peek_type(scanner) != Token_EndArray)
	{
		array.values[current_element] = parse_json_value(arena, scanner);
		++current_element;
		if (Scanner_peek_type(scanner) == Token_ValueSeparator)
		{
			scanner->cur++;
		}
		else
		{
			Scanner_expect_type(scanner, Token_EndArray);
		}
	}
	scanner->cur++;
	return array;
}

JSON_object parse_JSON_object(Arena* arena, Scanner* scanner);
json_value parse_json_value(Arena* arena, Scanner* scanner)
{
	json_value val;
	printf("parse_json_value\n");
	switch (Scanner_peek_type(scanner))
	{
		case Token_BeginObject:
			printf("JSON_object\n");
			JSON_object o = parse_JSON_object(arena, scanner);
			val.typ = JSONValue_Object;
			val.object = o;
			break;
		case Token_BeginArray:
			printf("JSON_array\n");
			JSON_array a = parse_JSON_array(arena, scanner);
			val.typ = JSONValue_Array;
			val.array = a;
			break;
		case Token_String:
			printf("JSON_string\n");
			JSON_string s = parse_JSON_string(arena, scanner);
			val.typ = JSONValue_String;
			val.string = s;
			break;
		case Token_Number:
			printf("JSON_number\n");
			JSON_number n = parse_JSON_number(scanner);
			val.typ = JSONValue_Number;
			val.number = n;
			break;
		case Token_True:
			val.typ = JSONValue_True;
			[[fallthrough]];
		case Token_False:
			val.typ = JSONValue_False;
			[[fallthrough]];
		case Token_Null:
			printf("JSON_[true, false, null]\n");
			val.typ = JSONValue_Null;
			scanner->cur++;
			break;
		default:
			assert(0);
	}
	return val;
}

JSON_object parse_JSON_object(Arena* arena, Scanner* scanner)
{
	Scanner_expect_type(scanner, Token_BeginObject);
	scanner->cur++;
	JSON_object object = {0};
	object.len = Scanner_object_len(scanner);
	if (object.len > 0)
	{
		void* buf = Arena_alloc(arena, sizeof(JSON_string) * object.len + sizeof(json_value) * object.len);
		object.keys = (JSON_string*)buf;
		object.values = (json_value*)(object.keys + object.len);
	}
	u32 current_member = 0;

	while (Scanner_peek_type(scanner) != Token_EndObject)
	{
		Scanner_expect_type(scanner, Token_String);
		JSON_string key = parse_JSON_string(arena, scanner);
		Scanner_expect_type(scanner, Token_NameSeparator);
		printf("[%d/%d] - (Key [%d])<%s>\n", current_member, object.len, key.len, key.str);
		scanner->cur++;
		object.keys[current_member] = key;
		object.values[current_member] = parse_json_value(arena, scanner);
		++current_member;
		if (Scanner_peek_type(scanner) == Token_ValueSeparator)
		{
			scanner->cur++;
		}
		else
		{
			Scanner_expect_type(scanner, Token_EndObject);
		}
	}
	scanner->cur++;
	return object;
}

JSON_object parse(Arena* arena, char* buf, tokens tokens)
{
	Scanner scanner = {0};
	scanner.buf = buf;
	scanner.tokens = tokens;

	return parse_JSON_object(arena, &scanner);
}

int main(int argc, char** argv)
{
	argc--;argv++;

	FILE* in = fopen(*argv, "r");
	if (!in)
	{
		perror("file:");
		return 1;
	}
	char file[4096];
	fread(file, 4096, 1, in);
	fclose(in);

	Arena	permarena = Arena_init(4096);
	Arena	temparena = Arena_init(4096);
	tokens	tokens = lex(&temparena, file);

	for (u32 i = 0; i < tokens.len; ++i)
	{
		int iter = tokens.starts[i];
		switch (tokens.types[i])
		{
			case Token_BeginObject:
				printf("Token_BeginObject [%d]\n", iter);
				break;
			case Token_EndObject:
				printf("Token_EndObject [%d]\n", iter);
				break;
			case Token_BeginArray:
				printf("Token_BeginArray [%d]\n", iter);
				break;
			case Token_EndArray:
				printf("Token_EndArray [%d]\n", iter);
				break;
			case Token_NameSeparator:
				printf("Token_NameSeparator [%d]\n", iter);
				break;
			case Token_ValueSeparator:
				printf("Token_ValueSeparator [%d]\n", iter);
				break;
			case Token_String:
				printf("Start String [%d]\n", iter);
				break;
			case Token_Number:
				printf("Start Number [%d]\n", iter);
				break;
			case Token_True:
				printf("Token_True [%d]\n", iter);
				break;
			case Token_False:
				printf("Token_False [%d]\n", iter);
				break;
			case Token_Null:
				printf("Token_Null [%d]\n", iter);
				break;
			default:
				break;
		}
	}
	parse(&permarena, file, tokens);

	Arena_deinit(&permarena);
	Arena_deinit(&temparena);
}
