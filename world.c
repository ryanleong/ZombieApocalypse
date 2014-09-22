#include "world.h"
#include "common.h"
#include "utils.h"
#include "random.h"

// local util functions
static void initTile(Tile *tile);
static void resetTile(Tile * tile);
static void destroyTile(Tile * tile);

World * newWorld(unsigned int width, unsigned int height) {
	World * world = (World *) checked_malloc(sizeof(World));

	world->clock = 0;
	world->width = width;
	world->height = height;

	world->xStart = 2;
	world->xEnd = width + 1;
	world->yStart = 2;
	world->yEnd = height + 1;

	world->stats = NO_STATS;

#ifdef _OPENMP
	world->locks = (omp_lock_t *) checked_malloc(sizeof(omp_lock_t) * (width + 4));
#endif
	world->map = (Tile **) checked_malloc(sizeof(Tile *) * (width + 4));
	for (unsigned int x = 0; x < width + 4; x++) {
		world->map[x] = (Tile *) checked_malloc(sizeof(Tile) * (height + 4));
		for (unsigned int y = 0; y < height + 4; y++) {
			Tile * tile = GET_TILE(world, x, y);
			initTile(tile);
			if (x <= 1 || y <= 1 || x >= width + 2 || y >= height + 2) {
				tile->type = BORDER;
			}
		}
#ifdef _OPENMP
		omp_init_lock(world->locks + x);
#endif
	}

	return world;
}

void resetWorld(World * world) {
#ifdef _OPENMP
#pragma omp parallel for collapse(2) schedule(guided, 10)
#endif
	for (int x = 0; x < world->width + 4; x++) {
		for (int y = 0; y < world->height + 4; y++) {
			Tile * tile = GET_TILE(world, x, y);
			resetTile(tile);
		}
	}
	world->stats = NO_STATS;
}

void destroyWorld(World * world) {
	for (unsigned int x = 0; x < world->width + 4; x++) {
		for (unsigned int y = 0; y < world->height + 4; y++) {
			Tile * tile = GET_TILE(world, x, y);
			destroyTile(tile);
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

static void initTile(Tile * tile) {
	tile->entity = NULL;
	tile->type = REGULAR;
}

static void resetTile(Tile * tile) {
	if (tile->entity != NULL) {
		disposeEntity(tile->entity);
		tile->entity = NULL;
	}
}

static void destroyTile(Tile * tile) {
	resetTile(tile);
}

void lockColumn(World * world, int x) {
#ifdef _OPENMP
	for (int i = x - 1; i <= x + 1; i++) {
		omp_set_lock(world->locks + i);
	}
#endif
}

void unlockColumn(World * world, int x) {
#ifdef _OPENMP
	for (int i = x - 1; i <= x + 1; i++) {
		omp_unset_lock(world->locks + i);
	}
#endif
}

Tile * getFreeAdjacent(World * input, World * output, int x, int y) {
	Tile * t;
	int permutation = randomInt(0, RANDOM_BASIC_DIRECTIONS - 1);
	for (int i = 0; i < 4; i++) {
		Direction dir = random_basic_directions[permutation][i];
		if (GET_TILE_DIR(input, dir, x, y)->entity == NULL
				&& (t = GET_TILE_DIR(output, dir, x, y))->entity == NULL) {
			return t;
		}
	}
	return NULL;
}

void copyStats(World * world, Stats stats) {
#ifndef NCUMULATIVE_STATS
	world->stats = stats;
	// these are the global stats which we need to reset
	// we want to keep just the event-driven
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
}
