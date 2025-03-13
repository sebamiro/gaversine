
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

#define Prfl_Start Global_Profiler.tsc_start = ReadCPUTimer();

#ifdef PROFILE

typedef struct Prfl_Anchor
{
	u64			tsc_inclusive;
	u64 		tsc_exclusive;
	u64 		hit_count;
	const char*	label;
} Prfl_Anchor;

typedef struct Profiler
{
	Prfl_Anchor	anchors[4096];
	u64			tsc_start;
	u64			tsc_end;
} Profiler;
static Profiler Global_Profiler;
static u32		Global_index_parent_anchor;

typedef struct Prfl_Block
{
	u32			index_anchor;
	u32 		index_parent_anchor;
	u64 		tsc_start;
	u64 		tsc_old_inclusive;
	const char*	label;
} Prfl_Block;

static Prfl_Block timeBlockStart(const char* label, u32 index_anchor)
{
	Prfl_Block	res;

	res.label = label;
	res.tsc_start = ReadCPUTimer();
	res.index_anchor = index_anchor;
	res.index_parent_anchor = Global_index_parent_anchor;

	Global_index_parent_anchor = index_anchor;
	Prfl_Anchor* anchor = Global_Profiler.anchors + index_anchor;
	res.tsc_old_inclusive = anchor->tsc_inclusive;
	return res;
}

static void	timeBlockEnd(Prfl_Block* block)
{
	u64 tsc_elapsed = ReadCPUTimer() - block->tsc_start;
	Global_index_parent_anchor = block->index_parent_anchor;

	Prfl_Anchor* anchor = Global_Profiler.anchors + block->index_anchor;
	Prfl_Anchor* parent = Global_Profiler.anchors + block->index_parent_anchor;

	parent->tsc_exclusive -= tsc_elapsed;
	anchor->tsc_exclusive += tsc_elapsed;
	anchor->tsc_inclusive = block->tsc_old_inclusive + tsc_elapsed;
	++anchor->hit_count;
	anchor->label = block->label;
}


#define TimeBlock_Start(name) \
	Prfl_Block block_##name = timeBlockStart(#name, __COUNTER__ + 1);
#define TimeBlock_End(name) \
	timeBlockEnd(&block_##name);

#define TimeFunction_Start \
	Prfl_Block block_func = timeBlockStart(__func__, __COUNTER__ + 1);
#define TimeFunction_End \
	timeBlockEnd(&block_func)

static void Prfl_End()
{
	Global_Profiler.tsc_end = ReadCPUTimer();
	u64 cpu_freq = EstimateCPUTimeFreq();
	u64 elapsed_total = Global_Profiler.tsc_end - Global_Profiler.tsc_start;
	if (cpu_freq != 0)
	{
		printf("\nTotal Time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)elapsed_total / (f64)cpu_freq, cpu_freq);
	}
	for (u32 index_anchor = 0; index_anchor < ArrayCount(Global_Profiler.anchors); ++index_anchor)
	{
		Prfl_Anchor* anchor = Global_Profiler.anchors + index_anchor;
		if (anchor->tsc_inclusive > 0)
		{
			f64 percent = 100.0 * ((f64)anchor->tsc_exclusive / (f64)elapsed_total);
			printf("\t[%s(%llu)]: %llu (%.2f%%", anchor->label, anchor->hit_count, anchor->tsc_exclusive, percent);
			if (anchor->tsc_exclusive != anchor->tsc_inclusive)
			{
				f64 percent_children = 100.0 * ((f64)anchor->tsc_inclusive / (f64)elapsed_total);
				printf(", %.2f%% w/children", percent_children);
			}
			printf(")\n");
		}
	}
}

#else

typedef struct Profiler
{
	u64			tsc_start;
	u64			tsc_end;
} Profiler;
static Profiler Global_Profiler;

#define TimeBlock_Start(...)
#define TimeBlock_End(...)
#define TimeFunction_Start
#define TimeFunction_End

static void Prfl_End()
{
	Global_Profiler.tsc_end = ReadCPUTimer();
	u64 cpu_freq = EstimateCPUTimeFreq();
	u64 elapsed_total = Global_Profiler.tsc_end - Global_Profiler.tsc_start;
	if (cpu_freq != 0)
	{
		printf("\nTotal Time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)elapsed_total / (f64)cpu_freq, cpu_freq);
	}
}

#endif

