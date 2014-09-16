#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "random.h"

/**
 * Initializes the random generator.
 * This function must be called once when the program starts.
 * If seed is zero, the current time is used instead.
 */
void initRandom(unsigned int seed) {
	if (seed == 0) {
		seed = time(NULL);
	}
	srandom(seed);
	srand48(seed);
}

/**
 * Gaussian distribution with mean and standard deviation.
 */
simClock randomEvent(simClock mean, simClock stdDev) {
	double u1 = drand48(); //these are uniform(0,1) random doubles
	double u2 = drand48();
	double randStdNormal = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2); //random normal(0,1)
	double randNormal = mean + stdDev * randStdNormal; //random normal(mean,stdDev^2)
	return randNormal;
}

/**
 * Both bounds are inclusive.
 */
int randomInt(int min, int max) {
	return random() % (max - min + 1) + min;
}

/**
 * Returns random double in range [0, 1)
 */
double randomDouble() {
	return drand48();
}
