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
#include "log.h"
#include "mpistuff.h"

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
	world->stats.clock = clock;
	world->stats.infectedFemales = 0;
	world->stats.infectedMales = 0;

	for (int i = 0; i < people;) {
		int x = randomInt(world->xStart, world->xEnd);
		int y = randomInt(world->yStart, world->yEnd);
		CellPtr cellPtr = GET_CELL_PTR(world, x, y);
		if (cellPtr->type != NONE) {
			continue;
		}

		newHuman(cellPtr, clock);
		if (cellPtr->gender == FEMALE) {
			world->stats.humanFemales++;
		} else {
			world->stats.humanMales++;
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
		world->stats.zombies++;

		i++;
	}
}

/**
 * Generates a dump for the world describing each entity.
 */
void printWorld(WorldPtr world) {
	// FIXME use global world
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
	fprintf(out, "Width %d; Height %d; Time %lld; Entities %d\n",
			world->localWidth, world->localHeight, world->clock, entities);

	for (int y = 0; y <= world->localHeight; y++) {
		for (int x = 0; x <= world->localWidth; x++) {
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

int divideArea(int width, int height, int parts) {
	int bestScore = 1 << 30;
	int bestColumns = 1;

	for (int i = 1; i < parts; i++) {
		int j = parts / i;
		if (i * j != parts) {
			continue;
		}

		int score = width / i + height / j;
		if (score < bestScore) {
			bestScore = score;
			bestColumns = i;
		}
	}
	return bestColumns;
}

int sizeOfPart(int size, int parts, int part) {
	return (part + 1) * size / parts - part * size / parts;
}

double divideWorld(int * width, int * height, WorldPtr * input,
		WorldPtr * output) {
	int columns, rows, x, y;
#ifdef USE_MPI
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	columns = divideArea(*width, *height, size);
	rows = size / columns;
	x = rank % columns;
	y = rank / columns;
#else
	columns = 1;
	rows = 1;
	x = 0;
	y = 0;
#endif
	int newWidth = sizeOfPart(*width, columns, x);
	int newHeight = sizeOfPart(*height, rows, y);

	WorldPtr worlds[2] = { newWorld(newWidth, newHeight), newWorld(newWidth,
			newHeight) };

	LOG_DEBUG("World size is %d x %d at position [%d, %d] of %d x %d ",
			newWidth, newHeight, x, y, columns, rows);

	for (int i = 0; i < 2; i++) {
		WorldPtr w = worlds[i];
		w->globalColumns = columns;
		w->globalRows = rows;
		w->globalWidth = *width;
		w->globalHeight = *height;
		w->globalX = x;
		w->globalY = y;
	}

	*input = worlds[0];
	*output = worlds[1];

	double ratio = newWidth * newHeight / (double) (*width * *height);

	*width = newWidth;
	*height = newHeight;
	return ratio;
}

int main(int argc, char **argv) {
	if (argc != 5) {
		LOG_ERROR("I want width, height, zombies, iterations.\n");
		exit(1);
	}

#ifdef USE_MPI
	MPI_Init(&argc, &argv);
#endif

	int width = atoi(argv[1]);
	int height = atoi(argv[2]);

	int people = (int) (width * height * INITIAL_DENSITY);
	int zombies = atoi(argv[3]);

	int iters = atoi(argv[4]);

	initRandom(0);

	WorldPtr input, output;
	double ratio = divideWorld(&width, &height, &input, &output);

	randomDistribution(input, people * ratio, zombies, 0);

#ifndef NIMAGES
	printWorld(input);
#endif

#ifdef TIME
	struct timeval t1, t2;
	double elapsedTime;
	gettimeofday(&t1, NULL);
#endif

	// FIXME cumulative stats
	for (int i = 0; i < iters; i++) {
		simulateStep(input, output);
		finishStep(output);
		printStatistics(output);

		Stats stats = output->stats;
		WorldPtr temp = input;
		input = output;
		output = temp;
		input->stats = stats;
		resetWorld(output);
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
	LOG_TIME("Simulation took %f milliseconds with %d threads\n", elapsedTime, numThreads);
#endif

	// this is a clean up
	// we destroy both worlds
	destroyWorld(input);
	destroyWorld(output);

	destroyRandom();

#ifdef USE_MPI
	MPI_Finalize();
#endif
}
