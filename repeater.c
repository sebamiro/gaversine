enum test_status
{
	TestState_Initialising,
	TestState_Testing,
	TestState_Completed,
	TestState_Error,
};

typedef struct Repeater_result
{
	u64	count_tests;
	u64	time_total;
	u64	time_max;
	u64	time_min;
} Repeater_result;

typedef struct Repeater_tester
{
	u64	cpu_freq;
	u64	time_try_for;
	u64	time_test_started;

	enum test_status status;
	u64	time_accumulated_on_test;
	u64	bytes_accumulated_on_test;

	Repeater_result	results;
} Repeater_tester;

void Repeater_begin_time(Repeater_tester* t)
{
	t->time_accumulated_on_test -= ReadCPUTimer();
}

void Repeater_end_time(Repeater_tester* t)
{
	t->time_accumulated_on_test += ReadCPUTimer();
}

void Repeater_count_bytes(Repeater_tester* t, u64 count_bytes)
{
	t->bytes_accumulated_on_test += count_bytes;
}

void print_time(char const *Label, f64 CPUTime, u64 CPUTimerFreq, u64 ByteCount);
u8	Repeater_is_testing(Repeater_tester* t)
{
	if (t->status != TestState_Testing)
	{
		return 0;
	}
	if (t->time_accumulated_on_test == 0)
	{
		return 1;
	}

	u64 time_current = ReadCPUTimer();
	u64 time_elapsed = t->time_accumulated_on_test;
	Repeater_result* results = &t->results;
	++results->count_tests;
	results->time_total += time_elapsed;
	if (results->time_max < time_elapsed)
	{
		results->time_max = time_elapsed;
	}
	if (results->time_min > time_elapsed)
	{
		t->time_test_started = time_current;
		results->time_min = time_elapsed;
		print_time("Min", time_elapsed, t->cpu_freq, t->bytes_accumulated_on_test);
		printf("\n");
	}

	t->time_accumulated_on_test = 0;
	t->bytes_accumulated_on_test = 0;

	if ((time_current - t->time_test_started) > t->time_try_for)
	{
		t->status = TestState_Completed;
		printf("Completed: TODO PRINT RESULTS\n");
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

void print_time(char const *Label, f64 CPUTime, u64 CPUTimerFreq, u64 ByteCount)
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
