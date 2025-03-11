#define Prfl_Start u64 prfl_start = ReadCPUTimer();

const char* prfl_names[4096];
u64	times[4096];
u8	len_prfl = 0;

#define TimeFunction_Start \
	u64 index_func_prf = len_prfl++; \
	prfl_names[index_func_prf] = __FUNCTION__; \
	times[index_func_prf] = ReadCPUTimer();

#define TimeFunction_End \
	times[index_func_prf] = ReadCPUTimer() - times[index_func_prf];

#define TimeBlock_Start(name) \
	u64 index_prf_##name = len_prfl++; \
	prfl_names[index_prf_##name] = #name; \
	times[index_prf_##name] = ReadCPUTimer();

#define TimeBlock_End(name) \
	times[index_prf_##name] = ReadCPUTimer() - times[index_prf_##name];

#define Prfl_End \
	u64 prfl_elapsed = ReadCPUTimer() - prfl_start;\
	u64 estimated_cpu_time_freq = EstimateCPUTimeFreq(); \
	if (estimated_cpu_time_freq != 0) \
	{ \
		printf("Time: %ld (%ldms)\n", prfl_elapsed, (prfl_elapsed * 1000) / estimated_cpu_time_freq); \
	} \
	for (u8 i = 0; i < len_prfl; i++) \
	{ \
		printf("\t[%s]: %ld (%%%ld)\n", prfl_names[i], times[i], times[i] * 100 / prfl_elapsed); \
	}

#ifdef linux
# include <x86intrin.h>
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

#else
# undef TimeFunction_Start
# define TimeFunction_Start
# undef TimeFunction_End
# define TimeFunction_End
# undef TimeBlock_Start
# define TimeBlock_Start(...)
# undef TimeBlock_End
# define TimeBlock_End(...)
# undef Prfl_Start
# define Prfl_Start
# undef Prfl_End
# define Prfl_End
#endif

