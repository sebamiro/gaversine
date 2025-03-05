#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef int64_t		i64;
typedef uint64_t	u64;
typedef int32_t		i32;
typedef uint32_t	u32;
typedef uint8_t		u8;

typedef enum type_token
{
	Token_None = 0x0,

	// structural
	Token_BeginObject = 0x7B, // { begin-object
	Token_EndObject = 0x7D, // } end-object
	Token_BeginArray = 0x5B, // [ begin-array
	Token_EndArray = 0x5D, // ] end-array
	Token_NameSeparator = 0x3A, // : name-separator
	Token_ValueSeparator = 0x2C, // , value-separator

	// whitespace
	Token_Space = 0x20,
	Token_Tab = 0x09,
	Token_Newline = 0x0A,
	Token_CarrigeReturn = 0x0D,

	// Values
	Token_True,
	Token_False,
	Token_Null,
	Token_String,
	Token_Number,
} type_token;

u8 is_whitespace(char c)
{
	return c == Token_Space ||
		c == Token_Tab ||
		c == Token_Newline ||
		c == Token_CarrigeReturn;
}

typedef struct Arena
{
	u8*	buf;
	u32 cur;
	u32 len;
} Arena;

Arena Arena_init(u32 len)
{
	Arena arena;
	arena.buf = malloc(sizeof(u8) * len);
	arena.len = len;
	arena.cur = 0;
	assert(arena.buf);
	return arena;
}

void* Arena_alloc(Arena* arena, u32 size)
{
	assert(size + arena->cur < arena->len);
	void* res = arena->buf + arena->cur;
	arena->cur += size;
	return res;
}

void Arena_deinit(Arena* arena)
{
	free(arena->buf);
}

typedef struct tokens
{
	type_token*	types;
	u32*		starts;
	u32			len;
} tokens;

tokens lex(Arena* arena, u8* file)
{
	u32 iter = 0;
	void* buf = Arena_alloc(arena, sizeof(type_token) * 100 + sizeof(u32) * 100);

	u32 current_token = 0;
	tokens tokens;
	tokens.len = 100;
	tokens.types = (type_token*)buf;
	tokens.starts = (u32*)(tokens.types + 100);

	while(file[iter] != 0)
	{
		while (file[iter] != 0 && is_whitespace(file[iter]))
		{
			++iter;
		}
		if (file[iter] == '"')
		{
			++iter;
			tokens.starts[current_token] = iter;
			tokens.types[current_token] = Token_String;
			++current_token;
			while(file[iter] != 0 && file[iter] != '"')
			{
				++iter;
			}
		}
		else if (file[iter] > 0x2F && file[iter] < 0x3A)
		{
			tokens.starts[current_token] = iter;
			tokens.types[current_token] = Token_Number;
			++current_token;
			while(file[iter] != 0 && ((file[iter] > 0x2F && file[iter] < 0x3A) || file[iter] == '.'))
			{
				++iter;
			}
		}
		else if ((file[iter] > 0x60 && file[iter] < 0x7B) || (file[iter] > 0x40 && file[iter] < 0x5b))
		{
			if (strncmp((char*)(file + iter), "true", 4) == 0)
			{
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_True;
				++current_token;
				iter += 4;
			}
			else if (strncmp((char*)(file + iter), "false", 5) == 0)
			{
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_False;
				++current_token;
				iter += 5;
			}
			else if (strncmp((char*)(file + iter), "null", 4) == 0)
			{
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_Null;
				++current_token;
				iter += 5;
			}
			else
			{
				assert(0);
			}
		}
		switch (file[iter])
		{
			case Token_BeginObject:
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_BeginObject;
				++current_token;
				break;
			case Token_EndObject:
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_EndObject;
				++current_token;
				break;
			case Token_BeginArray:
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_BeginArray;
				++current_token;
				break;
			case Token_EndArray:
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_EndArray;
				++current_token;
				break;
			case Token_NameSeparator:
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_NameSeparator;
				++current_token;
				break;
			case Token_ValueSeparator:
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_ValueSeparator;
				++current_token;
				break;
			default:
				break;
		}
		++iter;
	}
	return tokens;
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
	u8 file[4096];
	fread(file, 4096, 1, in);
	fclose(in);
	Arena	permarena = Arena_init(4096);
	tokens tokens = lex(&permarena, file);
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
}
