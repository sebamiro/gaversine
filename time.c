
#ifdef __linux__
# include <x86intrin.h>
# include <sys/time.h>

#define ArrayCount(x) ((sizeof(x)/sizeof(0[x])) / ((u64)(!(sizeof(x) % sizeof(0[x])))))

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

typedef struct Prfl_Anchor
{
	u64			tsc_elapsed;
	u64 		tsc_children_elapsed;
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
	const char*	label;
} Prfl_Block;

Prfl_Block timeBlockStart(const char* label, u32 index_anchor)
{
	Prfl_Block	res;

	res.label = label;
	res.tsc_start = ReadCPUTimer();
	res.index_anchor = index_anchor;
	res.index_parent_anchor = Global_index_parent_anchor;
	Global_index_parent_anchor = index_anchor;
	return res;
}

void	timeBlockEnd(Prfl_Block* block)
{
	u64 tsc_elapsed = ReadCPUTimer() - block->tsc_start;
	Global_index_parent_anchor = block->index_parent_anchor;

	Prfl_Anchor* anchor = Global_Profiler.anchors + block->index_anchor;
	Prfl_Anchor* parent = Global_Profiler.anchors + block->index_parent_anchor;

	anchor->tsc_elapsed = tsc_elapsed;
	parent->tsc_children_elapsed += tsc_elapsed;
	++anchor->hit_count;
	anchor->label = block->label;
}

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define Prfl_Start Global_Profiler.tsc_start = ReadCPUTimer();


#define TimeBlock_Start(name) Prfl_Block NameConcat(block, name) = timeBlockStart(#name, __COUNTER__ + 1);
#define TimeBlock_End(name) timeBlockEnd(&NameConcat(block, name));

#define TimeFunction_Start TimeBlock_Start(__FUNCTION__)
#define TimeFunction_End TimeBlock_End(__FUNCTION__)

static void Prfl_End()
{
	Global_Profiler.tsc_end = ReadCPUTimer();
	u64 cpu_freq = EstimateCPUTimeFreq();
	u64 elapsed_total = Global_Profiler.tsc_end - Global_Profiler.tsc_start;
	if (cpu_freq != 0)
	{
		printf("\nTotal Time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)elapsed_total / (f64)cpu_freq, cpu_freq);
	}
	for (u32 index_anchor = 0; index_anchor < ArrayCount(Global_Profiler.anchors); ++index_anchor)
	{
		Prfl_Anchor* anchor = Global_Profiler.anchors + index_anchor;
		if (anchor->tsc_elapsed > 0)
		{
			u64 elapsed = anchor->tsc_elapsed - anchor->tsc_children_elapsed;
			f64 percent = 100.0 * ((f64)elapsed / (f64)elapsed_total);
			printf("\t[%s(%lu)]: %ld (%.2f%%", anchor->label, anchor->hit_count, elapsed, percent);
			if (anchor->tsc_children_elapsed > 0)
			{
				f64 percent_children = 100.0 * ((f64)anchor->tsc_children_elapsed / (f64)elapsed_total);
				printf(", %.2f%%", percent_children);
			}
			printf(")\n");
		}
	}
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
# define Prfl_End()
#endif

