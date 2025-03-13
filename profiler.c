#define Prfl_Start Global_Profiler.tsc_start = ReadCPUTimer();

#ifdef PROFILE

typedef struct Prfl_Anchor
{
	u64			tsc_inclusive;
	u64 		tsc_exclusive;
	u64 		hit_count;
	u64 		count_processed_bytes;
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

static Prfl_Block timeBlockStart(const char* label, u32 index_anchor, u32 bytes_count)
{
	Prfl_Block	res;

	res.label = label;
	res.tsc_start = ReadCPUTimer();
	res.index_anchor = index_anchor;
	res.index_parent_anchor = Global_index_parent_anchor;

	Global_index_parent_anchor = index_anchor;
	Prfl_Anchor* anchor = Global_Profiler.anchors + index_anchor;
	res.tsc_old_inclusive = anchor->tsc_inclusive;
	anchor->count_processed_bytes += bytes_count;
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
	Prfl_Block block_##name = timeBlockStart(#name, __COUNTER__ + 1, 0);
#define TimeBlock_End(name) \
	timeBlockEnd(&block_##name);

#define TimeFunction_Start \
	Prfl_Block block_func = timeBlockStart(__func__, __COUNTER__ + 1, 0);
#define TimeFunction_End \
	timeBlockEnd(&block_func)

#define TimeBandwidth_Start(name, count) \
	Prfl_Block bandwidth_##name = timeBlockStart(#name, __COUNTER__ + 1, count);
#define TimeBandwidth_End(name) \
	timeBlockEnd(&bandwidth_##name);

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
		if (anchor->tsc_inclusive > 0)
		{
			f64 percent = 100.0 * ((f64)anchor->tsc_exclusive / (f64)elapsed_total);
			printf("\t[%s(%lu)]: %lu (%.2f%%", anchor->label, anchor->hit_count, anchor->tsc_exclusive, percent);
			if (anchor->tsc_exclusive != anchor->tsc_inclusive)
			{
				f64 percent_children = 100.0 * ((f64)anchor->tsc_inclusive / (f64)elapsed_total);
				printf(", %.2f%% w/children", percent_children);
			}

			if(anchor->count_processed_bytes)
			{
				f64 megabyte = 1024.0f*1024.0f;
				f64 gigabyte = megabyte*1024.0f;

				f64 seconds = (f64)anchor->tsc_inclusive / (f64)cpu_freq;
				f64 bps = (f64)anchor->count_processed_bytes / seconds;
				f64 megabytes = (f64)anchor->count_processed_bytes / megabyte;
				f64 gps = bps / gigabyte;

				printf("  %.3fmb at %.2fgb/s", megabytes, gps);
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
#define TimeBandwidth_Start(...)
#define TimeBandwidth_End(...)

static void Prfl_End()
{
	Global_Profiler.tsc_end = ReadCPUTimer();
	u64 cpu_freq = EstimateCPUTimeFreq();
	u64 elapsed_total = Global_Profiler.tsc_end - Global_Profiler.tsc_start;
	if (cpu_freq != 0)
	{
		printf("\nTotal Time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)elapsed_total / (f64)cpu_freq, cpu_freq);
	}
}

#endif
