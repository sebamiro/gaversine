#include <sys/resource.h>

#ifdef __linux__
# include <x86intrin.h>
#else
// SPDX-License-Identifier: GPL-2.0
static u64 __rdtsc(void)
{
    u64 val;

    /*
     * According to ARM DDI 0487F.c, from Armv8.0 to Armv8.5 inclusive, the
     * system counter is at least 56 bits wide; from Armv8.6, the counter
     * must be 64 bits wide.  So the system counter could be less than 64
     * bits wide and it is attributed with the flag 'cap_user_time_short'
     * is true.
     */
    __asm volatile("mrs %0, cntvct_el0" : "=r" (val));

    return val;
}
#endif

# include <sys/time.h>

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

u64 ReadOsPageFaults(void)
{
	struct rusage rusage;
	getrusage(RUSAGE_SELF, &rusage);
	u64 res = rusage.ru_minflt + rusage.ru_majflt;
	return res;
}

