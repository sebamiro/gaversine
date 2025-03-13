#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "type.h"
#include "tokens.h"
#include "JSON.h"

#include "time.c"
#include "Arena.c"
#include "lex.c"
#include "parse.c"
#include "haversine.c"

int main(int argc, char** argv)
{
	Prfl_Start;

	argc--;argv++;
	if (argc < 1)
	{
		return 1;
	}

	FILE* in = fopen(*argv, "r");
	if (!in)
	{
		perror("file:");
		return 1;
	}
	fseek(in, 0L, SEEK_END);
	long file_size = ftell(in);

	TimeBandwidth_Start(Read, file_size);
	char* file = malloc(file_size / sizeof(char));
	rewind(in);
	fread(file, sizeof(char), file_size, in);
	TimeBandwidth_End(Read);

	f64*	check = NULL;
	u32		len_check;
	if (argv[1])
	{
		FILE* in_check = fopen(argv[1], "r");
		if (!in_check)
		{
			perror("file:");
			return 1;
		}
		fseek(in_check, 0L, SEEK_END);
		len_check = (u32)(ftell(in_check));
		check = malloc(len_check);
		rewind(in_check);
		fread(check, sizeof(f64), len_check, in_check);
		fclose(in_check);
	}

	Arena	permarena = Arena_init(4096);
	tokens	tokens = lex(file);
	JSON json = parse(&permarena, file, tokens);

	TimeBlock_Start(CleanParse);
	free(tokens.types);
	free(tokens.starts);
	free(file);
	TimeBlock_End(CleanParse);



	assert(json.values[0].typ == JSONValue_Object);
	assert(json.values[1].typ == JSONValue_String);
	assert(json.values[2].typ == JSONValue_Array);
	JSON_array array = json.values[2].array;
	f64 total = 0;
	TimeBandwidth_Start(Sum, array.len * sizeof(JSON_value));
	for (u32 i = 0; i < array.len; i++)
	{
		assert(json.values[array.values[i]].typ == JSONValue_Object);
		JSON_object pair = json.values[array.values[i]].object;
		assert(pair.len == 4);
		f64 x0 = json.values[pair.values[0]].number.num_float;
		f64 y0 = json.values[pair.values[1]].number.num_float;
		f64 x1 = json.values[pair.values[2]].number.num_float;
		f64 y1 = json.values[pair.values[3]].number.num_float;

		f64 haversine = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
		if (check)
		{
			if (haversine != check[i])
			{
				fprintf(stdout, "[%d] %.16f != %.16f\n", i, haversine, check[i]);
			}
		}
		total += haversine;
	}
	if (check)
	{
		if (total != check[array.len])
		{
			fprintf(stdout, "TOTAL: %f != %f", total, check[array.len]);
		}
	}
	TimeBandwidth_End(Sum);

	TimeBlock_Start(MiscEnd);
	fprintf(stdout, "Avg: %f\n", total / array.len);
	Arena_deinit(&permarena);
	TimeBlock_End(MiscEnd);

	Prfl_End();
}
