#define Prfl_Start Global_Profiler.tscStart = ReadCPUTimer();

#ifdef PROFILE

typedef struct prfl_anchor
{
	u64			tscInclusive;
	u64 		tscExclusive;
	u64 		count_Hit;
	u64 		count_ProcessedBytes;
	const char*	label;
} prfl_anchor;

typedef struct Profiler
{
	prfl_anchor	anchors[4096];
	u64			tscStart;
	u64			tscEnd;
} Profiler;

static Profiler Global_Profiler;
static u32		Global_index_parent_anchor;

typedef struct prfl_block
{
	u32			index_Anchor;
	u32 		index_ParentAnchor;
	u64 		tscStart;
	u64 		tscOldInclusive;
	const char*	label;
} prfl_block;

static prfl_block timeBlockStart(const char* label, u32 index_Anchor, u32 bytes_count)
{
	prfl_block	res;

	res.label = label;
	res.tscStart = ReadCPUTimer();
	res.index_Anchor = index_Anchor;
	res.index_ParentAnchor = Global_index_parent_anchor;

	Global_index_parent_anchor = index_Anchor;
	prfl_anchor* anchor = Global_Profiler.anchors + index_Anchor;
	res.tscOldInclusive = anchor->tscInclusive;
	anchor->count_ProcessedBytes += bytes_count;
	return res;
}

static void	timeBlockEnd(prfl_block* block)
{
	u64 tscElapsed = ReadCPUTimer() - block->tscStart;
	Global_index_parent_anchor = block->index_ParentAnchor;

	prfl_anchor* anchor = Global_Profiler.anchors + block->index_Anchor;
	prfl_anchor* parent = Global_Profiler.anchors + block->index_ParentAnchor;

	parent->tscExclusive -= tscElapsed;
	anchor->tscExclusive += tscElapsed;
	anchor->tscInclusive = block->tscOldInclusive + tscElapsed;
	++anchor->count_Hit;
	anchor->label = block->label;
}


#define TimeBlock_Start(name) \
	prfl_block block_##name = timeBlockStart(#name, __COUNTER__ + 1, 0);
#define TimeBlock_End(name) \
	timeBlockEnd(&block_##name);

#define TimeFunction_Start \
	prfl_block block_func = timeBlockStart(__func__, __COUNTER__ + 1, 0);
#define TimeFunction_End \
	timeBlockEnd(&block_func)

#define TimeBandwidth_Start(name, count) \
	prfl_block bandwidth_##name = timeBlockStart(#name, __COUNTER__ + 1, count);
#define TimeBandwidth_End(name) \
	timeBlockEnd(&bandwidth_##name);

static void Prfl_End()
{
	Global_Profiler.tscEnd = ReadCPUTimer();
	u64 cpuFreq = EstimateCPUTimeFreq();
	u64 time_ElapsedTotal = Global_Profiler.tscEnd - Global_Profiler.tscStart;
	if (cpuFreq != 0)
	{
		printf("\nTotal Time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)time_ElapsedTotal / (f64)cpuFreq, cpuFreq);
	}
	for (u32 index_Anchor = 0; index_Anchor < ArrayCount(Global_Profiler.anchors); ++index_Anchor)
	{
		prfl_anchor* anchor = Global_Profiler.anchors + index_Anchor;
		if (anchor->tscInclusive > 0)
		{
			f64 percent = 100.0 * ((f64)anchor->tscExclusive / (f64)time_ElapsedTotal);
			printf("\t[%s(%lu)]: %lu (%.2f%%", anchor->label, anchor->count_Hit, anchor->tscExclusive, percent);
			if (anchor->tscExclusive != anchor->tscInclusive)
			{
				f64 percentChildren = 100.0 * ((f64)anchor->tscInclusive / (f64)time_ElapsedTotal);
				printf(", %.2f%% w/children", percentChildren);
			}

			if(anchor->count_ProcessedBytes)
			{
				f64 megabyte = 1024.0f*1024.0f;
				f64 gigabyte = megabyte*1024.0f;

				f64 seconds = (f64)anchor->tscInclusive / (f64)cpuFreq;
				f64 bps = (f64)anchor->count_ProcessedBytes / seconds;
				f64 megabytes = (f64)anchor->count_ProcessedBytes / megabyte;
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
	u64			tscStart;
	u64			tscEnd;
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
	Global_Profiler.tscEnd = ReadCPUTimer();
	u64 cpuFreq = EstimateCPUTimeFreq();
	u64 time_ElapsedTotal = Global_Profiler.tscEnd - Global_Profiler.tscStart;
	if (cpuFreq != 0)
	{
		printf("\nTotal Time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)time_ElapsedTotal / (f64)cpuFreq, cpuFreq);
	}
}

#endif
