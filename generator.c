#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "type.h"

#include "haversine.c"

int Usage()
{
	fprintf(stderr, "USAGE: ./haversine-generator seed count\n");
	return 2;
}

f64 RandomFloat(f64 max, f64 min)
{
	return min + (f64)rand() / ((f64)RAND_MAX / (max - min));
}


int main(int argc, char** argv)
{
	--argc; ++argv;
	if (argc != 2 && argv)
	{
		return Usage();
	}
	int seed, count;

	seed = atoi(argv[0]);
	count = atoi(argv[1]);
	srand(seed);

	char outName[30], dataOutName[30];
	snprintf(outName, 30, "gaversine_%s_%s.json", argv[0], argv[1]);
	snprintf(dataOutName, 30, "gaversine_%s_%s.data", argv[0], argv[1]);

	FILE* out, *dataOut;
	out = fopen(outName, "w");
	dataOut = fopen(dataOutName, "w");

	int chunk = count / 10;
	f64 total = 0;

	fprintf(out, "{\n\"pairs\": [\n");
	for (int i = 0; i < count; i += chunk)
	{
		f64 maxX = RandomFloat(180, -180);
		f64 minX = RandomFloat(180, -180);
		f64 maxY = RandomFloat(90, -90);
		f64 minY = RandomFloat(90, -90);
		for (int j = 0; j < chunk; ++j)
		{
			f64 x0 = RandomFloat(maxX, minX);
			f64 x1 = RandomFloat(maxX, minX);
			f64 y0 = RandomFloat(maxY, minY);
			f64 y1 = RandomFloat(maxY, minY);

			f64 haversine = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
			total += haversine;

			fprintf(out, "\t{ \"x0\": %.16f, \"y0\": %.16f, \"x1\": %.16f, \"y1\": %.16f }", x0, y0, x1, y1);
			fwrite(&haversine, sizeof(haversine), 1, dataOut);
			if (j + 1 < chunk)
			{
				fprintf(out, ",\n");
			}
		}
		if (i + chunk < count)
		{
			fprintf(out, ",\n");
		}
		else
		{
			fprintf(out, "\n]}");
		}
	}

	fwrite(&total, sizeof(total), 1, dataOut);
	printf("\tAvg: %f\n", total / count);
	fclose(out);
	fclose(dataOut);
	return 0;
}

