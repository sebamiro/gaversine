#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "type.h"

#include "time.c"
#include "repeater.c"

void test_mmap_full(repeater_tester* t, char* file_name, u64 sizeFile)
{
	u64* buf;
	while (Repeater_IsTesting(t))
	{
		buf = mmap(NULL, sizeFile, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, -1, 0);
		assert(buf != MAP_FAILED);
		FILE* in = fopen(file_name, "r");
		if (!in)
		{
			perror("file");
			return;
		}
		Repeater_BeginTime(t);
		fread(buf, sizeFile, 1, in);
		Repeater_EndTime(t);
		Repeater_CountBytes(t, sizeFile);
		fclose(in);
		munmap(buf, sizeFile);
	}
}





void test_mmap_chunks(repeater_tester* t, char* file_name, u64 sizeChunk, u64 sizeFile)
{
	u64* buf;
	while (Repeater_IsTesting(t))
	{
		buf = mmap(NULL, sizeChunk, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, -1, 0);
		assert(buf != MAP_FAILED);
		FILE* in = fopen(file_name, "r");
		if (!in)
		{
			perror("file");
			return;
		}
		Repeater_BeginTime(t);
		for (u64 i = 1; i * sizeChunk < sizeFile; i++)
		{
			fread(buf, sizeChunk, 1, in);
		}
		Repeater_EndTime(t);
		Repeater_CountBytes(t, sizeFile);
		fclose(in);
		munmap(buf, sizeChunk);
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
	u64 sizeFile = stats.st_size;

	repeater_tester	tester = {0};
	while (1)
	{
		for (int i = 0; i < 2; i++)
		{
			tester.cpuFreq = EstimateCPUTimeFreq();
			tester.time_TryFor = 10 * tester.cpuFreq;
			tester.time_TestStarted = ReadCPUTimer();
			tester.status = TestState_Testing;
			tester.results.time_Min = -1;
			if (i == 0)
			{
				printf("\n--- mmap full ---\n");
				test_mmap_full(&tester, *argv, sizeFile);
			}
			else {
				for (u64 sizeChunk = 256*1024; sizeChunk < sizeFile; sizeChunk*=2)
				{
					tester.cpuFreq = EstimateCPUTimeFreq();
					tester.time_TryFor = 10 * tester.cpuFreq;
					tester.time_TestStarted = ReadCPUTimer();
					tester.status = TestState_Testing;
					tester.results.time_Min = -1;
					printf("\n--- mmap chunk %ldmk ---\n", sizeChunk);
					test_mmap_chunks(&tester, *argv,  sizeChunk, sizeFile);
				}
			}
		}
	}
}
