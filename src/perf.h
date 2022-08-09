#include "global.h"
#include <time.h>

struct Performance {
	struct timespec frameStart;
	uint64_t frameSleep;
	double load;
};

[[maybe_unused]] static struct Performance perf_init() {
	return (struct Performance){{0, 0}, 0, 0};
}

[[maybe_unused]] static uint64_t DeltaNs(struct timespec from, struct timespec to) {
	if(to.tv_nsec < from.tv_nsec)
		to.tv_sec -= 1, to.tv_nsec += 1000000000llu;
	return (to.tv_sec - from.tv_sec) * 1000000000llu + (to.tv_nsec - from.tv_nsec);
}

[[maybe_unused]] static void perf_tick(struct Performance *perf, struct timespec sleepStart, struct timespec sleepEnd) {
	perf->frameSleep += DeltaNs(sleepStart, sleepEnd);
	uint64_t frameTotal = DeltaNs(perf->frameStart, sleepEnd);
	if(frameTotal >= 1000000000llu) {
		double load = (double)(frameTotal - perf->frameSleep) / frameTotal;
		perf->frameStart = sleepEnd;
		perf->frameSleep = 0;
		perf->load = (perf->load + load) / 2;
		uprintf("load: %f (norm %f)\n", load, perf->load);
	}
}
