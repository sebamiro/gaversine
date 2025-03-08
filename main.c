#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "type.h"
#include "tokens.h"
#include "JSON.h"

#include "Arena.c"
#include "lex.c"
#include "parse.c"

int main(int argc, char** argv)
{
	argc--;argv++;

	FILE* in = fopen(*argv, "r");
	if (!in)
	{
		perror("file:");
		return 1;
	}
	fseek(in, 0L, SEEK_END);
	long file_size = ftell(in);
	char* file = malloc(file_size * sizeof(char));
	rewind(in);

	fread(file, sizeof(char), file_size, in);

	Arena	permarena = Arena_init(4096);
	tokens	tokens = lex(file);
	JSON json = parse(&permarena, file, tokens);
	free(tokens.types);
	free(tokens.starts);
	free(file);
	(void)json;

	assert(json.values[0].typ == JSONValue_Object);
	assert(json.values[1].typ == JSONValue_String);
	assert(json.values[2].typ == JSONValue_Array);
	JSON_array array = json.values[2].array;
	for (u32 i = 0; i < array.len; i++)
	{
		assert(json.values[array.values[i]].typ == JSONValue_Object);
		JSON_object pair = json.values[array.values[i]].object;
		assert(pair.len == 4);
		f64 x0 = json.values[pair.values[0]].number.num_float;
		f64 y0 = json.values[pair.values[1]].number.num_float;
		f64 x1 = json.values[pair.values[2]].number.num_float;
		f64 y1 = json.values[pair.values[3]].number.num_float;
		printf("x0: %f, y0: %f | x1: %f, y1: %f\n", x0, y0, x1, y1);
	}
	Arena_deinit(&permarena);
}
