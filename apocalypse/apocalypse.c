#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "world.h"
#include "random.h"
#include "simulation.h"
#include "constants.h"
#include "log.h"
#include "mpistuff.h"
#include "communication.h"
#include "output.h"
#include "stats.h"

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

int main(int argc, char **argv) {
#ifdef USE_MPI
	MPI_Init(&argc, &argv);
#endif

	if (argc != 5) {
		LOG_ERROR("I want width, height, zombies, iterations.\n");
#ifdef USE_MPI
		MPI_Finalize();
#endif
		exit(1);
	}

	int width = atoi(argv[1]);
	int height = atoi(argv[2]);

	int people = (int) (width * height * INITIAL_DENSITY);
	int zombies = atoi(argv[3]);

	int iters = atoi(argv[4]);

	initRandom(0);

	WorldPtr input, output;
	double ratio = divideWorld(&width, &height, &input, &output);

	// there should not be any output prior to this point
#ifdef REDIRECT
	initRedirectToFiles(input);
#endif

	LOG_DEBUG("World size is %d x %d at position [%d, %d] of %d x %d\n",
			input->localWidth, input->localHeight, input->globalX,
			input->globalY, input->globalColumns, input->globalRows);

	if (input->globalX == 0 && input->globalY == 0) {
		randomDistribution(input, people * ratio, zombies, 0);
	} else {
		// no zombies elsewhere
		randomDistribution(input, people * ratio, 0, 0);
	}

#ifndef NIMAGES
	printWorld(input, false);
#endif

#ifdef TIME
	Timer timer = startTimer();
#endif

	Stats cumulative = NO_STATS;
	for (int i = 0; i < iters; i++) {
		simulateStep(input, output);

		output->stats.clock = cumulative.clock = output->clock;
		Stats stats = output->stats;
		mergeStats(&cumulative, stats, false);
		printStatistics(output, cumulative);

		WorldPtr temp = input;
		input = output;
		output = temp;
		input->stats = stats;
	}

#ifdef TIME
	double elapsedTime = getElapsedTime(timer);

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

#ifdef REDIRECT
	finishRedirectToFiles();
#endif

#ifdef USE_MPI
	MPI_Finalize();
#endif
}
