/*
 * random.h
 *
 *  Created on: Sep 1, 2014
 *      Author: adam
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include "clock.h"

void initRandom(unsigned int seed);

simClock randomEvent(simClock expected, simClock deviation);

int randomNumber(int min, int max);

#endif /* RANDOM_H_ */
