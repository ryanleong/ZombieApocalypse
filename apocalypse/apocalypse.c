#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef TIME
#include <sys/time.h>
#endif

#include "world.h"
#include "entity.h"
#include "random.h"
#include "simulation.h"
#include "common.h"
#include "constants.h"
#include "common.h"

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
 * Fills the world with specified number of people and zombies.
 * The people are of different age; zombies are "brand new".
 */
void randomDistribution(WorldPtr world, int people, int zombies, simClock clock) {
	for (int i = 0; i < people;) {
		int x = randomInt(world->xStart, world->xEnd);
		int y = randomInt(world->yStart, world->yEnd);
		CellPtr cellPtr = GET_CELL_PTR(world, x, y);
		if (cellPtr->type != NONE) {
			continue;
		}

		newHuman(cellPtr, clock);
		if (cellPtr->gender == FEMALE) {
			world->lastStats.humanFemales++;
		} else {
			world->lastStats.humanMales++;
		}

		i++;
	}

	for (int i = 0; i < zombies;) {
		int x = randomInt(world->xStart, world->xEnd);
		int y = randomInt(world->yStart, world->yEnd);
		CellPtr cellPtr = GET_CELL_PTR(world, x, y);
		if (cellPtr->type != NONE) {
			continue;
		}

		newZombie(cellPtr, clock);
		world->lastStats.zombies++;

		i++;
	}
}

/**
 * Generates a dump for the world describing each entity.
 */
void printWorld(WorldPtr world) {
	char gender[5] = { 'M', 'F', 'f', 'f', 'f' };

	char filename[255];
	sprintf(filename, "images/step-%06lld.img", world->clock);

	FILE * out = fopen(filename, "w");
	if (out == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		return;
	}

	int entities = world->stats.humanFemales + world->stats.humanMales
			+ world->stats.infectedFemales + world->stats.infectedMales
			+ world->stats.zombies;
	fprintf(out, "Width %d; Height %d; Time %lld; Entities %d\n", world->width,
			world->height, world->clock, entities);

	for (int y = 0; y <= world->height; y++) {
		for (int x = 0; x <= world->width; x++) {
			CellPtr ptr = &GET_CELL(world, x + world->xStart,
					y + world->yStart);
			int age = world->clock - ptr->origin;
			switch (ptr->type) {
			case NONE:
				// nothing
				break;
			case HUMAN:
				fprintf(out, "[%d %d] H %c %d\n", x, y,
						gender[ptr->gender + ptr->children], age);
				break;
			case INFECTED:
				fprintf(out, "[%d %d] I %c %d\n", x, y,
						gender[ptr->gender + ptr->children], age);
				break;
			case ZOMBIE:
				fprintf(out, "[%d %d] Z _ %d\n", x, y, age);
			}
		}
	}

	fclose(out);
}

/**
 *  Print the number of humans, infected people (who carry the disease, but
 *  haven't yet become zombies), and zombies, for debugging.
 */
void printPopulations(WorldPtr world) {
	Stats stats = world->stats;
	// make sure there are always blanks around numbers
	// that way we can easily split the line
	printf("Time: %6lld \tHumans: %6d \tInfected: %6d \tZombies: %6d\n",
			world->clock, stats.humanFemales + stats.humanMales,
			stats.infectedFemales + stats.infectedMales, stats.zombies);

#ifndef NDETAILED_STATS
	printf("LHF: %6d \tLHM: %6d \tLIF: %6d \tLIM: %6d \tLZ:  %6d\n",
			stats.humanFemales, stats.humanMales, stats.infectedFemales,
			stats.infectedMales, stats.zombies);
	printf("DHF: %6d \tDHM: %6d \tDIF: %6d \tDIM: %6d \tDZ:  %6d\n",
			stats.humanFemalesDied, stats.humanMalesDied,
			stats.infectedFemalesDied, stats.infectedMalesDied,
			stats.zombiesDecomposed);
	printf("BHF: %6d \tBHM: %6d \tBIF: %6d \tBIM: %6d\n",
			stats.humanFemalesBorn, stats.humanMalesBorn,
			stats.infectedFemalesBorn, stats.infectedMalesBorn);
	printf("PH:  %6d \tPI:  %6d \tGBH: %6d \tGBI: %6d \tCML: %6d \tCC:  %6d\n",
			stats.humanFemalesPregnant, stats.infectedFemalesPregnant,
			stats.humanFemalesBecameInfected, stats.infectedFemalesGivingBirth,
			stats.couplesMakingLove, stats.childrenConceived);
	printf("IHF: %6d \tIHM: %6d \tIFZ: %6d \tIMZ: %6d\n",
			stats.humanFemalesBecameInfected, stats.humanMalesBecameInfected,
			stats.infectedFemalesBecameZombies,
			stats.infectedMalesBecameZombies);
#endif
}

void printStatistics(WorldPtr world) {
#ifndef NIMAGES
	if (world->clock % IMAGES_EVERY == 0) {
		printWorld(world);
	}
#endif

#ifndef NPOPULATION
	if (world->clock % POPULATION_EVERY == 0) {
		printPopulations(world);
	}
#endif
}

int main(int argc, char **argv) {
	if (argc != 5) {
		printf("I want width, height, zombies, iterations.\n");
		exit(1);
	}

	unsigned int width = atoi(argv[1]);
	unsigned int height = atoi(argv[2]);

	unsigned int people = (int) (width * height * INITIAL_DENSITY);
	unsigned int zombies = atoi(argv[3]);

	unsigned int iters = atoi(argv[4]);

	initRandom(0);

	WorldPtr input = newWorld(width, height);
	WorldPtr output = newWorld(width, height);

	randomDistribution(input, people, zombies, 0);
#ifndef NIMAGES
	// we need to fake stats
	input->stats = input->lastStats;
	printWorld(input);
#endif

#ifdef TIME
	struct timeval t1, t2;
	double elapsedTime;
	gettimeofday(&t1, NULL);
#endif

	for (int i = 0; i < iters; i++) {
		simulateStep(input, output);
		finishStep(output);
		printStatistics(output);

		Stats stats = output->stats;
		WorldPtr temp = input;
		input = output;
		output = temp;
		input->lastStats = stats;
		resetWorld(output);
		copyStats(output, stats);
	}

#ifdef TIME
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; // sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;// us to ms

#ifdef _OPENMP
	int numThreads = omp_get_max_threads();
#else
	int numThreads = 1;
#endif
	printf("Took %f milliseconds with %d threads\n", elapsedTime, numThreads);
#endif

	// this is a clean up
	// we destroy both worlds
	destroyWorld(input);
	destroyWorld(output);

	destroyRandom();
}
