#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "type.h"

#include "haversine.c"

int usage()
{
	fprintf(stderr, "USAGE: ./haversine-generator seed count\n");
	return 2;
}

f64 random_float(f64 max, f64 min)
{
	return min + (f64)rand() / ((f64)RAND_MAX / (max - min));
}


int main(int argc, char** argv)
{
	--argc; ++argv;
	if (argc != 2 && argv)
	{
		return usage();
	}
	int seed = atoi(argv[0]);
	int count = atoi(argv[1]);
	srand(seed);

	char out_name[30];
	char data_out_name[30];
	snprintf(out_name, 30, "gaversine_%s_%s.json", argv[0], argv[1]);
	snprintf(data_out_name, 30, "gaversine_%s_%s.data", argv[0], argv[1]);

	FILE* out = fopen(out_name, "w");
	FILE* data_out = fopen(data_out_name, "w");

	int chunk = count / 10;

	f64 total = 0;

	fprintf(out, "{\n\"pairs\": [\n");
	for (int i = 0; i < count; i += chunk)
	{
		f64 max_x = random_float(180, -180);
		f64 min_x = random_float(180, -180);
		f64 max_y = random_float(90, -90);
		f64 min_y = random_float(90, -90);
		for (int j = 0; j < chunk; ++j)
		{
			f64 x0 = random_float(max_x, min_x);
			f64 x1 = random_float(max_x, min_x);
			f64 y0 = random_float(max_y, min_y);
			f64 y1 = random_float(max_y, min_y);

			f64 haversine = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
			total += haversine;

			fprintf(out, "\t{ \"x0\": %.16f, \"y0\": %.16f, \"x1\": %.16f, \"y1\": %.16f }", x0, y0, x1, y1);
			fwrite(&haversine, sizeof(haversine), 1, data_out);
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

	fwrite(&total, sizeof(f64), 1, data_out);
	printf("\tAvg: %f\n", total / count);
	fclose(out);
	fclose(data_out);
	return 0;
}

