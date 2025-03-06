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

JSON_string parse_JSON_string(Arena* arena, Scanner* scanner)
{
	JSON_string string;

	string.len = Scanner_string_len(scanner);
	string.str = (char*)Arena_alloc(arena, sizeof(char) * string.len);
	strncpy(string.str, scanner->buf + scanner->tokens.starts[scanner->cur], string.len);
	scanner->cur++;
	return string;
}

JSON_object parse_JSON_object(Arena* arena, Scanner* scanner)
{
	Scanner_expect_type(scanner, Token_BeginObject);
	scanner->cur++;
	JSON_object object = {0};

	while (Scanner_peek_type(scanner) != Token_EndObject)
	{
		if (Scanner_peek_type(scanner) == Token_String)
		{
			JSON_string key = parse_JSON_string(arena, scanner);
			printf("(String [%d])<%s>\n", key.len, key.str);
		}
		scanner->cur++;
	}
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
