/*
 * output.h
 *
 *  Created on: Oct 13, 2014
 *      Author: adam
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "world.h"
#include "stats.h"

#ifndef OUTPUT_EVERY
#define OUTPUT_EVERY 1
#endif

#ifndef IMAGES_EVERY
#define IMAGES_EVERY OUTPUT_EVERY
#endif

#if IMAGES_EVERY <= 0 && ! defined(NIMAGES)
#define NIMAGES
#endif

#ifndef POPULATION_EVERY
#define POPULATION_EVERY OUTPUT_EVERY
#endif

#if POPULATION_EVERY <= 0 && ! defined(NPOPULATION)
#define NIMAGES
#endif

/**
 * Generates a dump for the world describing each entity.
 */
void printWorld(WorldPtr world, bool borders);

/**
 *  Print the number of humans, infected people (who carry the disease, but
 *  haven't yet become zombies), and zombies, for debugging.
 */
void printPopulations(Stats stats);

void printStatistics(WorldPtr world, Stats cumulative);

#endif /* OUTPUT_H_ */
