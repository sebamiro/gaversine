#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef double f64;

static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */

    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 Result = EarthRadius * c;

    return Result;
}

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

			fprintf(out, "\t{ \"x0\": %f, \"y0\": %f, \"x1\": %f, \"y1\": %f }", x0, y0, x1, y1);
			fprintf(data_out, "%f", haversine);
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

	fprintf(data_out, "%f", total);
	printf("\tAvg: %f\n", total / count);
	fclose(out);
	fclose(data_out);
	return 0;
}

