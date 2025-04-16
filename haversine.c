#include <math.h>

static f64 Square(f64 a)
{
    f64 res = (a*a);
    return res;
}

static f64 RadiansFromDegrees(f64 degrees)
{
    f64 res = 0.01745329251994329577 * degrees;
    return res;
}

f64 maxSin = 0;
f64 minSin = 0;
f64 maxCos = 0;
f64 minCos = 0;
f64 maxSqrt = 0;
f64 minSqrt = 0;

// NOTE(casey): earthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 earthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */

    f64 lat1 = y0;
    f64 lat2 = y1;
    f64 lon1 = x0;
    f64 lon2 = x1;

    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

	if (dLat/2.0 < minSin)
	{
		minSin = dLat/2.0;
	}
	if (dLat/2.0 > maxSin)
	{
		maxSin = dLat/2.0;
	}

	if (lat1 < minCos)
	{
		minCos = lat1;
	}
	if (lat2 < minCos)
	{
		minCos = lat2;
	}
	if (lat1 > maxCos)
	{
		maxCos = lat1;
	}
	if (lat2 > maxCos)
	{
		maxCos = lat2;
	}

	if (sin(dLat/2.0) < minSqrt)
	{
		minSqrt = sin(dLat/2.0);
	}
	if (sin(dLat/2.0) > maxSqrt)
	{
		maxSqrt = sin(dLat/2.0);
	}

    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 res = earthRadius * c;

    return res;
}
