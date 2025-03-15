enum test_status
{
	TestState_Initialising,
	TestState_Testing,
	TestState_Completed,
	TestState_Error,
};

typedef struct repeater_result
{
	u64	count_Tests;
	u64	time_Total;
	u64	time_Max;
	u64	time_Min;
} repeater_result;

typedef struct repeater_tester
{
	u64	cpuFreq;
	u64	time_TryFor;
	u64	time_TestStarted;

	enum test_status status;
	u64	time_AccumulatedOnTest;
	u64	bytes_AccumulatedOnTest;
	u64	pageFaults_AccumulatedOnTest;

	repeater_result	results;
} repeater_tester;

void Repeater_BeginTime(repeater_tester* t)
{
	t->time_AccumulatedOnTest -= ReadCPUTimer();
	t->pageFaults_AccumulatedOnTest -= ReadOsPageFaults();
}

void Repeater_EndTime(repeater_tester* t)
{
	t->time_AccumulatedOnTest += ReadCPUTimer();
	t->pageFaults_AccumulatedOnTest += ReadOsPageFaults();
}

void Repeater_CountBytes(repeater_tester* t, u64 count_bytes)
{
	t->bytes_AccumulatedOnTest += count_bytes;
}

void printTime(char const *Label, f64 CPUTime, u64 CPUTimerFreq, u64 ByteCount);
u8	Repeater_IsTesting(repeater_tester* t)
{
	if (t->status != TestState_Testing)
	{
		return 0;
	}
	if (t->time_AccumulatedOnTest == 0)
	{
		return 1;
	}

	u64 time_Current = ReadCPUTimer();
	u64 time_Elapsed = t->time_AccumulatedOnTest;
	repeater_result* results = &t->results;
	++results->count_Tests;
	results->time_Total += time_Elapsed;
	if (results->time_Max < time_Elapsed)
	{
		results->time_Max = time_Elapsed;
	}
	if (results->time_Min > time_Elapsed)
	{
		t->time_TestStarted = time_Current;
		results->time_Min = time_Elapsed;
		printTime("Min", time_Elapsed, t->cpuFreq, t->bytes_AccumulatedOnTest);
		if (t->pageFaults_AccumulatedOnTest)
		{
			printf(" PF: %0.4f (%0.4fk/fault)", (f64)t->pageFaults_AccumulatedOnTest, (f64)t->bytes_AccumulatedOnTest / ((f64)t->pageFaults_AccumulatedOnTest * 1024.0));
		}
		printf("\n");
	}

	t->time_AccumulatedOnTest = 0;
	t->bytes_AccumulatedOnTest = 0;
	t->pageFaults_AccumulatedOnTest = 0;

	if ((time_Current - t->time_TestStarted) > t->time_TryFor)
	{
		t->status = TestState_Completed;
		printf("\nCompleted %ld tests in %ld:\n", results->count_Tests, results->time_Total);
		printTime("Min", results->time_Min, t->cpuFreq, t->bytes_AccumulatedOnTest);
		printf("\n");
		printTime("Max", results->time_Max, t->cpuFreq, t->bytes_AccumulatedOnTest);
		printf("\n");
		printf(" PF: %0.4f (%0.4fk/fault)", (f64)t->pageFaults_AccumulatedOnTest, (f64)t->bytes_AccumulatedOnTest / ((f64)t->pageFaults_AccumulatedOnTest * 1024.0));
		printf("\n");
	}
	return t->status == TestState_Testing;
}

static f64 SecondsFromCPUTime(f64 CPUTime, u64 CPUTimerFreq)
{
    f64 Result = 0.0;
    if(CPUTimerFreq)
    {
        Result = (CPUTime / (f64)CPUTimerFreq);
    }

    return Result;
}

void printTime(char const *Label, f64 CPUTime, u64 CPUTimerFreq, u64 ByteCount)
{
    printf("%s: %.0f", Label, CPUTime);
    if(CPUTimerFreq)
    {
        f64 Seconds = SecondsFromCPUTime(CPUTime, CPUTimerFreq);
        printf(" (%fms)", 1000.0f*Seconds);

        if(ByteCount)
        {
            f64 Gigabyte = (1024.0f * 1024.0f * 1024.0f);
            f64 BestBandwidth = ByteCount / (Gigabyte * Seconds);
            printf(" %fgb/s", BestBandwidth);
        }
    }
}
