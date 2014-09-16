#include "world.h"
#include "common.h"
#include "utils.h"

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
	}

	return world;
}

void resetWorld(World * world) {
#ifdef _OPENMP
	int threads = omp_get_max_threads();
	int numThreads = MIN(MAX(world->width * world->height / 10, 1), threads);
// each thread resets at least 10 elements
#pragma omp parallel for default(shared) num_threads(numThreads)
#endif
	for (int x = 0; x < world->width + 4; x++) {
		for (int y = 0; y < world->height + 4; y++) {
			Tile * tile = GET_TILE(world, x, y);
			resetTile(tile);
		}
	}
}

void destroyWorld(World * world) {
	for (unsigned int x = 0; x < world->width + 4; x++) {
		for (unsigned int y = 0; y < world->height + 4; y++) {
			Tile * tile = GET_TILE(world, x, y);
			destroyTile(tile);
		}
		free(world->map[x]);
	}
	free(world->map);
	free(world);
}

static void initTile(Tile * tile) {
	tile->entity = NULL;
	tile->type = REGULAR;
#ifdef _OPENMP
	omp_init_lock(&tile->lock);
#endif
}

static void resetTile(Tile * tile) {
	if (tile->entity != NULL) {
		disposeEntity(tile->entity);
		tile->entity = NULL;
	}
}

static void destroyTile(Tile * tile) {
	resetTile(tile);
#ifdef _OPENMP
	omp_destroy_lock(&tile->lock);
#endif
}

void lockTile(Tile * tile) {
#ifdef _OPENMP
	omp_set_lock(&tile->lock);
#endif
}

void unlockTile(Tile * tile) {
#ifdef _OPENMP
	omp_unset_lock(&tile->lock);
#endif
}

Tile * getFreeAdjacent(World * input, World * output, int x, int y) {
	Tile * t;
	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		if (GET_TILE_DIR(input, dir, x, y)->entity == NULL
				&& (t = GET_TILE_DIR(output, dir, x, y))->entity == NULL) {
			return t;
		}
	}
	return NULL;
}
