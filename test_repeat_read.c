#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

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
			if (!buf)
			{
				buf = malloc(file_size / sizeof(char));
			}
			Repeater_begin_time(t);
			fread(buf, file_size, 1, in);
			Repeater_end_time(t);
			Repeater_count_bytes(t, file_size);
			fclose(in);
			free(buf);
			buf = NULL;
		}
	}
}

void test_read(Repeater_tester* t, char* buf, char* file_name, long file_size)
{
	while (Repeater_is_testing(t))
	{
		int in = open(file_name, O_RDONLY);
		if (in > 0)
		{
			if (!buf)
			{
				buf = malloc(file_size / sizeof(char));
			}
			Repeater_begin_time(t);
			read(in, buf, file_size);
			Repeater_end_time(t);
			Repeater_count_bytes(t, file_size);
			close(in);
			free(buf);
			buf = NULL;
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

	free(buf);
	buf = NULL;
	Repeater_tester	tester = {0};
	while (1)
	{
		for (int i = 0; i < 2; i++)
		{
			tester.cpu_freq = EstimateCPUTimeFreq();
			tester.time_try_for = 10 * tester.cpu_freq;
			tester.time_test_started = ReadCPUTimer();
			tester.status = TestState_Testing;
			tester.results.time_min = -1;
			if (i) {
				printf("\n--- Read ---\n");
				test_read(&tester, buf, *argv, file_size);
			} else {
				printf("\n--- Fread ---\n");
				test_fread(&tester, buf, *argv, file_size);
			}
		}
	}
}
