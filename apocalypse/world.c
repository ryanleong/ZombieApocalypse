#include "world.h"
#include "common.h"
#include "utils.h"
#include "random.h"

WorldPtr newWorld(unsigned int width, unsigned int height) {
	WorldPtr world = (WorldPtr) malloc(sizeof(World));

	world->clock = 0;
	world->stats = NO_STATS;
	world->stats.clock = world->clock;
	world->stats.width = width;
	world->stats.height = height;

	world->globalSize[0] = 1;
	world->globalSize[1] = 1;
	world->globalPosition[0] = 0;
	world->globalPosition[1] = 0;
	world->globalWidth = width;
	world->globalHeight = height;

	world->localWidth = world->globalWidth;
	world->localHeight = world->globalHeight;

	world->xStart = 2;
	world->xEnd = width + 1;
	world->yStart = 2;
	world->yEnd = height + 1;

#ifdef _OPENMP
	world->locks = (omp_lock_t *) malloc(sizeof(omp_lock_t) * (width + 4));
#endif
	world->map1d = (Cell *) malloc(sizeof(Cell) * (width + 4) * (height + 4));
	world->map = (Cell **) malloc(sizeof(Cell *) * (width + 4));
	for (int x = 0; x < width + 4; x++) {
		world->map[x] = world->map1d + x * (height + 4);
		for (int y = 0; y < height + 4; y++) {
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
	for (int x = 0; x < world->localWidth + 4; x++) {
		for (int y = 0; y < world->localHeight + 4; y++) {
			GET_CELL(world, x, y).type = NONE;
		}
	}

	world->stats = NO_STATS;
	world->stats.width = world->localWidth;
	world->stats.height = world->localHeight;
}

void destroyWorld(WorldPtr world) {
#ifdef _OPENMP
	for (unsigned int x = 0; x < world->localWidth + 4; x++) {
		omp_destroy_lock(world->locks + x);
	}
#endif
	free(world->map1d);
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
