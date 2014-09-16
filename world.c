#include "world.h"
#include "common.h"

World * newWorld(unsigned int width, unsigned int height) {
	World * w = (World *) malloc(sizeof(World));

	w->clock = 0;
	w->width = width;
	w->height = height;
	w->map = (Tile *) calloc((width + 2) * (height + 2), sizeof(Tile));
	for (int i = 0; i < width + 2; i++) {
		for (int j = 0; j < height + 2; j++) {
			Tile * t = GET_TILE(w, i, j);
			initTile(t);
			if (i == 0 || j == 0 || i == width + 1 || j == height + 1) {
				t->type = BORDER;
			}
		}
	}

	return w;
}

void resetWorld(World * world) {
#ifdef _OPENMP
	int threads = omp_get_max_threads();
	int numThreads = MIN(MAX(world->width * world->height / 10, 1), threads);
	// each thread resets at least 10 elements
#pragma omp parallel for default(shared) num_threads(numThreads)
#endif
	for (int i = 0; i < (world->width + 2) * (world->height + 2); i++) {
		resetTile(world->map + i);
	}
}

void destoyWorld(World * world) {
	for (int i = 0; i < (world->width + 2) * (world->height + 2); i++) {
		destroyTile(world->map + i);
	}
	free(world->map);
	free(world);
}

void initTile(Tile * tile) {
	resetTile(tile);
	tile->type = REGULAR;
#ifdef _OPENMP
	omp_init_lock(&tile->lock);
#endif
}

void resetTile(Tile * tile) {
	if (tile->entity != NULL) {
		disposeEntity(tile->entity);
		tile->entity = NULL;
	}
}

void destroyTile(Tile * tile) {
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
