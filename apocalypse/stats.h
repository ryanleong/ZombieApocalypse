#ifndef STATS_H_
#define STATS_H_

#include <stdbool.h>

#include "clock.h"

typedef struct Stats {
	// world related
	simClock clock;
	int width;
	int height;

	// entities related
	int humanFemales;
	int humanMales;
	int infectedFemales;
	int infectedMales;
	int zombies;
	int humanFemalesDied;
	int humanMalesDied;
	int infectedFemalesDied;
	int infectedMalesDied;
	int zombiesDecomposed;
	int humanFemalesBorn;
	int humanMalesBorn;
	int humanFemalesGivingBirth;
	int humanFemalesPregnant;
	int infectedFemalesBorn;
	int infectedMalesBorn;
	int infectedFemalesGivingBirth;
	int infectedFemalesPregnant;
	int couplesMakingLove;
	int childrenConceived;
	int humanFemalesBecameInfected;
	int humanMalesBecameInfected;
	int infectedFemalesBecameZombies;
	int infectedMalesBecameZombies;
} Stats;

#define NO_STATS ((const struct Stats) {0})

void mergeStats(Stats * dest, Stats src, bool absolute);

#endif /* STATS_H_ */

// vim: ts=4 sw=4 et
