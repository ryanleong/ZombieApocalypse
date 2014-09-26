#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#include "random.h"

typedef unsigned short PRNGState[3];

static PRNGState *states;

void initRandom(unsigned int seed) {
	if (seed == 0) {
		seed = time(NULL);
	}
	srand48(seed);

#ifdef _OPENMP
	int threads = omp_get_max_threads();
#else
	int threads = 1;
#endif

	states = malloc(sizeof(PRNGState) * threads);
	for (int i = 0; i < threads; i++) {
		states[i][0] = (unsigned short) (0xFFFF * drand48());
		states[i][1] = (unsigned short) (0xFFFF * drand48());
		states[i][2] = (unsigned short) (0xFFFF * drand48());
	}
}

void destroyRandom() {
	free(states);
}

simClock randomEvent(simClock mean, simClock stdDev) {
	double u1 = randomDouble(); //these are uniform(0,1) random doubles
	double u2 = randomDouble();
	double randStdNormal = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2); //random normal(0,1)
	double randNormal = mean + stdDev * randStdNormal; //random normal(mean,stdDev^2)
	return randNormal;
}

int randomInt(int min, int max) {
	return ((int) (randomDouble() * (1 << 24))) % (max - min + 1) + min;
}

double randomDouble() {
#ifdef _OPENMP
	int thread = omp_get_thread_num();
#else
	int thread = 0;
#endif
	return erand48(states[thread]);
}
