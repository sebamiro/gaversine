#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "type.h"
#include "tokens.h"
#include "JSON.h"

#include "time.c"
#include "profiler.c"

#include "arena.c"
#include "lex.c"
#include "parse.c"
#include "haversine.c"

int main(int argc, char** argv)
{
	Prfl_Start;

	argc--;
	if (argc < 1)
	{
		fprintf(stderr, "USAGE: %s file [check]\n", *argv);
		return 1;
	}
	argv++;

	struct stat stats;
	if (stat(*argv, &stats))
	{
		perror("stat");
		return 1;
	}

	u64 sizeFile = stats.st_size;
	char* file  = mmap(NULL, sizeFile, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, -1, 0);
	assert(file != MAP_FAILED);

	FILE* in = fopen(*argv, "r");
	TimeBandwidth_Start(Read, sizeFile);
	if (fread(file, sizeFile, 1, in) == 0)
	{
		perror("file");
		return 1;
	}
	TimeBandwidth_End(Read);
	fclose(in);

	f64*	arrCheck = NULL;
	u32		sizeCheck;
	if (argv[1])
	{
		if (stat(argv[1], &stats))
		{
			perror("check file:");
			return 1;
		}
		sizeCheck = stats.st_size / sizeof(f64);
		arrCheck = malloc(sizeCheck * sizeof(f64));
		FILE* inCheck = fopen(argv[1], "r");
		if (fread(arrCheck, sizeCheck, sizeof(f64), inCheck) == 0)
		{
			perror("file");
			return 1;
		}
		fclose(inCheck);
	}

	arena	permArena = Arena_Init(Size_DefaultRegion);

	TimeBandwidth_Start(FullParse, sizeFile);
	tokens	tokens = Lex(file, sizeFile);
	json json = Parse(&permArena, file, tokens);
	TimeBandwidth_End(FullParse);

	TimeBlock_Start(CleanParse);
	free(tokens.Start);
	free(tokens.Type);
	munmap(file, sizeFile);
	TimeBlock_End(CleanParse);

	assert(json.Values[0].Type == JSONValue_Object);
	assert(json.Values[1].Type == JSONValue_String);
	assert(json.Values[2].Type == JSONValue_Array);

	json_array Array = json.Values[2].Array;
	f64 total = 0;
	TimeBandwidth_Start(Sum, Array.Len * 4 * sizeof(f64));
	for (u32 i = 0; i < Array.Len; i++)
	{
		assert(json.Values[Array.Values[i]].Type == JSONValue_Object);
		json_object pair = json.Values[Array.Values[i]].Object;
		assert(pair.Len == 4);
		f64 x0 = json.Values[pair.Values[0]].Number.Float;
		f64 y0 = json.Values[pair.Values[1]].Number.Float;
		f64 x1 = json.Values[pair.Values[2]].Number.Float;
		f64 y1 = json.Values[pair.Values[3]].Number.Float;

		f64 haversine = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
		if (arrCheck)
		{
			if (haversine != arrCheck[i])
			{
				fprintf(stdout, "[%d] %.16f != %.16f\n", i, haversine, arrCheck[i]);
			}
		}
		total += haversine;
	}
	if (arrCheck)
	{
		if (total != arrCheck[Array.Len])
		{
			fprintf(stdout, "TOTAL: %f != %f", total, arrCheck[Array.Len]);
		}
		else
		{
			fprintf(stdout, "TOTAL: %f == %f", total, arrCheck[Array.Len]);
		}
	}
	TimeBandwidth_End(Sum);

	TimeBlock_Start(MiscEnd);
	fprintf(stdout, "Avg: %f\n", total / Array.Len);
	Arena_deinit(&permArena);
	free(json.Values);
	TimeBlock_End(MiscEnd);


	Prfl_End();
}
