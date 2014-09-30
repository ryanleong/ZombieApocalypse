#include "world.h"
#include "common.h"
#include "utils.h"
#include "random.h"

WorldPtr newWorld(unsigned int width, unsigned int height) {
	WorldPtr world = (WorldPtr) checked_malloc(sizeof(World));

	world->clock = 0;
	world->width = width;
	world->height = height;

	world->xStart = 2;
	world->xEnd = width + 1;
	world->yStart = 2;
	world->yEnd = height + 1;

	world->stats = NO_STATS;
	world->stats.clock = 0;
	world->stats.width = width;
	world->stats.height = height;
	world->lastStats = NO_STATS;
	world->lastStats.clock = 0;
	world->lastStats.width = width;
	world->lastStats.height = height;

#ifdef _OPENMP
	world->locks = (omp_lock_t *) checked_malloc(sizeof(omp_lock_t) * (width + 4));
#endif
	world->map = (Cell **) checked_malloc(sizeof(Cell *) * (width + 4));
	for (unsigned int x = 0; x < width + 4; x++) {
		world->map[x] = (Cell *) checked_malloc(sizeof(Cell) * (height + 4));
		for (unsigned int y = 0; y < height + 4; y++) {
			GET_CELL(world, x, y).type = NONE;
		}
#ifdef _OPENMP
		omp_init_lock(world->locks + x);
#endif
	}

	return world;
}

void resetWorld(WorldPtr world) {
#ifdef _OPENMP
#pragma omp parallel for schedule(guided, 10) collapse(2)
#endif
	for (int x = 0; x < world->width + 4; x++) {
		for (int y = 0; y < world->height + 4; y++) {
			GET_CELL(world, x, y).type = NONE;
		}
	}

	world->stats = NO_STATS;
	world->stats.width = world->width;
	world->stats.height = world->height;
	world->lastStats = NO_STATS;
	world->lastStats.width = world->width;
	world->lastStats.height = world->height;
}

void destroyWorld(WorldPtr world) {
	for (unsigned int x = 0; x < world->width + 4; x++) {
		for (unsigned int y = 0; y < world->height + 4; y++) {
			GET_CELL(world, x, y).type = NONE;
		}
		free(world->map[x]);
#ifdef _OPENMP
		omp_destroy_lock(world->locks + x);
#endif
	}
	free(world->map);
#ifdef _OPENMP
	free(world->locks);
#endif
	free(world);
}

void lockColumn(WorldPtr world, int x) {
#ifdef _OPENMP
	for (int i = x - 1; i <= x + 1; i++) {
		omp_set_lock(world->locks + i);
	}
#endif
}

void unlockColumn(WorldPtr world, int x) {
#ifdef _OPENMP
	for (int i = x - 1; i <= x + 1; i++) {
		omp_unset_lock(world->locks + i);
	}
#endif
}

CellPtr getFreeAdjacent(WorldPtr input, WorldPtr output, int x, int y) {
	CellPtr freePtr;
	int permutation = randomInt(0, RANDOM_BASIC_DIRECTIONS - 1);
	for (int i = 0; i < 4; i++) {
		Direction dir = random_basic_directions[permutation][i];
		if (GET_CELL_DIR(input, dir, x, y).type == NONE && (freePtr =
				GET_CELL_PTR_DIR(output, dir, x, y))->type == NONE) {
			return freePtr;
		}
	}
	return NULL;
}

void copyStats(WorldPtr world, Stats stats) {
#ifndef NCUMULATIVE_STATS
	world->stats = stats;
	// these are the global stats which we need to reset
	// we want to keep just the event-driven
	world->stats.clock = 0;
	world->stats.humanFemales = 0;
	world->stats.humanMales = 0;
	world->stats.infectedFemales = 0;
	world->stats.infectedMales = 0;
	world->stats.zombies = 0;
	world->stats.humanFemalesPregnant = 0;
	world->stats.infectedFemalesPregnant = 0;
#else
	world->stats = NO_STATS;
#endif
	world->stats.width = world->width;
	world->stats.height = world->height;
}
