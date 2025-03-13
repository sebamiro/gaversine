#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>

#include "type.h"

#include "time.c"
#include "repeater.c"


void test_fread(Repeater_tester* t, char* buf, char* file_name, long file_size)
{
	while (Repeater_is_testing(t))
	{
		FILE* in = fopen(file_name, "r");
		if (in)
		{
			Repeater_begin_time(t);
			fread(buf, file_size, 1, in);
			Repeater_end_time(t);
			Repeater_count_bytes(t, file_size);
			fclose(in);
		}
	}
}

int main(int argc, char** argv)
{
	argc--;argv++;
	if (argc < 1)
	{
		return 1;
	}

	struct stat stats;
	stat(*argv, &stats);
	u64 file_size = stats.st_size;
	char* buf = malloc(file_size / sizeof(char));

	Repeater_tester	tester = {0};
	while (1)
	{
		tester.cpu_freq = EstimateCPUTimeFreq();
		tester.time_try_for = 10 * tester.cpu_freq;
		tester.time_test_started = ReadCPUTimer();
		tester.status = TestState_Testing;
		tester.results.time_min = -1;
		test_fread(&tester, buf, *argv, file_size);
	}
}
