#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "type.h"

f64 random_f64(f64 lo, f64 hi)
{
	return lo + (f64)rand() / (f64)RAND_MAX / (hi - lo);
}

int	main(int argc, char** argv)
{
	argc--;
	if (argc != 2)
	{
		fprintf(stderr, "USAGE: %s seed count\n", *argv);
		return 1;
	}
	argv++;
	u32 seed = atoi(argv[0]);
	u32 count = atoi(argv[1]);
	char file_name[64];
	sprintf(file_name, "gaversine_%d_%d.json", seed, count);
	srand(seed);
	FILE* file = fopen(file_name, "w");
	assert(file);
	u32 chunk = count / 10;

	fprintf(file, "{\n\t\"paris\": [\n");
	for (u32 i = 0; i < count; i += chunk)
	{
		f64 XLO = random_f64(-180, 180);
		f64 XHI = random_f64(-180, 180);
		f64 YLO = random_f64(-90, 90);
		f64 YHI = random_f64(-90, 90);

		for (u32 j = 0; j < chunk; j++)
		{
			f64 x0 = random_f64(XLO, XHI);
			f64 x1 = random_f64(XLO, XHI);
			f64 y0 = random_f64(YLO, YHI);
			f64 y1 = random_f64(YLO, YHI);
			fprintf(file, "\t\t{ \"x0\": %f, \"y0\": %f, \"x1\": %f, \"y1\": %f }", x0, y0, x1, y1);
			if (j + i + 1 < count)
			{
				fprintf(file, ",");
			}
			fprintf(file, "\n");
		}
	}
	fprintf(file, "\t]\n}\n");
}
