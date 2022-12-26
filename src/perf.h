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
	if(to.tv_nsec < from.tv_nsec) {
		to.tv_sec -= 1;
		to.tv_nsec += UINT32_C(1000000000);
	}
	return ((uint64_t)to.tv_sec - (uint64_t)from.tv_sec) * UINT64_C(1000000000) + ((uint64_t)to.tv_nsec - (uint64_t)from.tv_nsec);
}

[[maybe_unused]] static void perf_tick(struct Performance *perf, struct timespec sleepStart, struct timespec sleepEnd) {
	perf->frameSleep += DeltaNs(sleepStart, sleepEnd);
	uint64_t frameTotal = DeltaNs(perf->frameStart, sleepEnd);
	if(frameTotal >= UINT64_C(1000000000)) {
		double load = (double)(frameTotal - perf->frameSleep) / (double)frameTotal;
		perf->frameStart = sleepEnd;
		perf->frameSleep = 0;
		perf->load = (perf->load + load) / 2;
		uprintf("load: %f (norm %f)\n", load, perf->load);
	}
}
