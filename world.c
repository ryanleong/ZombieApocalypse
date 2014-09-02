#include "world.h"
#include "common.h"

World * newWorld(unsigned int width, unsigned int height) {
	World * w;
	w = (World *) malloc(sizeof(World));

	w->clock = 0;
	w->width = width;
	w->height = height;
	w->map = (Tile *) calloc(width * height, sizeof(Tile));
	for (int i = 0; i < width * height; i++) {
		initTile(w->map + i);
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
	for (int i = 0; i < world->width * world->height; i++) {
		resetTile(world->map + i);
	}
}

void destoyWorld(World * world) {
	for (int i = 0; i < world->width * world->height; i++) {
		destroyTile(world->map + i);
	}
	free(world->map);
	free(world);
}

void initTile(Tile * tile) {
	resetTile(tile);
#ifdef _OPENMP
	omp_init_lock(&tile->lock);
#endif
}

void resetTile(Tile * tile) {
	if (tile->living != NULL) {
		disposeEntities(tile->living->asEntity);
	}
	if (tile->zombies != NULL) {
		disposeEntities(tile->zombies->asEntity);
	}

	tile->living = NULL;
	tile->zombies = NULL;
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
