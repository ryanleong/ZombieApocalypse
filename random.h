/*
 * random.h
 *
 *  Created on: Sep 1, 2014
 *      Author: adam
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include "clock.h"

/**
 * Initializes the random generator.
 * This function must be called once when the program starts.
 * If seed is zero, the current time is used instead.
 */
void initRandom(unsigned int seed);

/**
 * Gaussian distribution with mean and standard deviation.
 */
simClock randomEvent(simClock mean, simClock stdDev);

/**
 * Returns a random integer in range with both bounds included.
 */
int randomInt(int min, int max);

/**
 * Returns random double in range [0, 1)
 */
double randomDouble();

#endif /* RANDOM_H_ */
