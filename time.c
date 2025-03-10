#include <x86intrin.h>
#include <sys/time.h>

 u64 GetOSTimerFreq(void)
{
	return 1000000;
}

static u64 ReadOSTimer(void)
{
	struct timeval Value;
	gettimeofday(&Value, 0);

	u64 Result = GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec;
	return Result;
}

u64 ReadCPUTimer(void)
{
	return __rdtsc();
}

static u64 EstimateCPUTimeFreq()
{
	u64 ms = 100;
	u64 os_freq = GetOSTimerFreq();

	u64 os_start = ReadOSTimer();
	u64 cpu_start = ReadCPUTimer();
	u64 os_end = 0;
	u64 os_elapsed = 0;
	u64 os_wait_time = os_freq * ms / 1000;
	while (os_elapsed < os_wait_time)
	{
		os_end = ReadOSTimer();
		os_elapsed = os_end - os_start;
	}

	u64 cpu_end = ReadCPUTimer();
	u64 cpu_elapsed = cpu_end - cpu_start;

	u64 cpu_freq = 0;
	if (os_elapsed)
	{
		cpu_freq = os_freq * cpu_elapsed / os_elapsed;
	}
	return cpu_freq;
}
