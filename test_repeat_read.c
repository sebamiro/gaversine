#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "type.h"

#include "time.c"
#include "repeater.c"


void test_fread(repeater_tester* t, char* buf, char* file_name, long file_size)
{
	while (Repeater_IsTesting(t))
	{
		FILE* in = fopen(file_name, "r");
		if (in)
		{
			if (!buf)
			{
				buf = malloc(file_size / sizeof(char));
			}
			Repeater_BeginTime(t);
			fread(buf, file_size, 1, in);
			Repeater_EndTime(t);
			Repeater_CountBytes(t, file_size);
			fclose(in);
			free(buf);
			buf = NULL;
		}
	}
}

void test_read(repeater_tester* t, char* buf, char* file_name, long file_size)
{
	while (Repeater_IsTesting(t))
	{
		int in = open(file_name, O_RDONLY);
		if (in > 0)
		{
			if (!buf)
			{
				buf = malloc(file_size / sizeof(char));
			}
			Repeater_BeginTime(t);
			read(in, buf, file_size);
			Repeater_EndTime(t);
			Repeater_CountBytes(t, file_size);
			close(in);
			free(buf);
			buf = NULL;
		}
	}
}

void test_buf(repeater_tester* t, char* buf, long file_size)
{
	while (Repeater_IsTesting(t))
	{
		Repeater_BeginTime(t);
		for (u32 i = 0; i < file_size; i++)
		{
			buf[i] = i;
		}
		Repeater_EndTime(t);
		Repeater_CountBytes(t, file_size);
	}
}

void test_malloc_buf(repeater_tester* t, char* buf, long file_size)
{
	while (Repeater_IsTesting(t))
	{
		buf = malloc(file_size);
		Repeater_BeginTime(t);
		for (u32 i = 0; i < file_size; i++)
		{
			buf[i] = i;
		}
		Repeater_EndTime(t);
		Repeater_CountBytes(t, file_size);
		free(buf);
	}
}

void test_buf_back(repeater_tester* t, char* buf, long file_size)
{
	while (Repeater_IsTesting(t))
	{
		Repeater_BeginTime(t);
		for (u32 i = 0; i < file_size; i++)
		{
			buf[file_size - 1 - i] = i;
		}
		Repeater_EndTime(t);
		Repeater_CountBytes(t, file_size);
	}
}

void test_malloc_buf_back(repeater_tester* t, char* buf, long file_size)
{
	while (Repeater_IsTesting(t))
	{
		buf = malloc(file_size);
		Repeater_BeginTime(t);
		for (u32 i = 0; i < file_size; i++)
		{
			buf[file_size - 1 - i] = i;
		}
		Repeater_EndTime(t);
		Repeater_CountBytes(t, file_size);
		free(buf);
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

	repeater_tester	tester = {0};
	while (1)
	{
		for (int i = 0; i < 4; i++)
		{
			tester.cpuFreq = EstimateCPUTimeFreq();
			tester.time_TryFor = 10 * tester.cpuFreq;
			tester.time_TestStarted = ReadCPUTimer();
			tester.status = TestState_Testing;
			tester.results.time_Min = -1;
			if (i == 0) {
				printf("\n--- Rev Malloc Buf ---\n");
				test_malloc_buf_back(&tester, NULL,  file_size);
			}
			else if (i == 1) {
				printf("\n--- Rev Buf ---\n");
				test_buf_back(&tester, buf, file_size);
			}
			else if (i == 2) {
				printf("\n--- Buf ---\n");
				test_buf(&tester, buf, file_size);
			}
			else if (i == 3) {
				printf("\n--- Malloc Buf ---\n");
				test_malloc_buf(&tester, NULL, file_size);
			}
		}
	}
}
