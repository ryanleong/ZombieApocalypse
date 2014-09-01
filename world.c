#include "world.h"

World * newWorld(unsigned int width, unsigned int height) {
	World * w;
	w = (World *) malloc(sizeof(World));

	w->clock = 0;
	w->width = width;
	w->height = height;
	w->map = (Tile **) calloc(width, sizeof(Tile*));
	for (int i = 0; i < width; i++) {
		w->map[i] = (Tile *) calloc(height, sizeof(Tile));
		for (int j = 0; j < height; j++) {
			initTile(&w->map[i][j]);
		}
	}

	return w;
}

void destoyWorld(World * world) {
	for (int i = 0; i < world->width; i++) {
		free(world->map[i]);
	}
	free(world->map);
	free(world);
}

void initTile(Tile * tile) {
	resetTile(tile); // so far nothing else is necessary
}

void resetTile(Tile * tile) {
	tile->living = NULL;
	tile->zombies = NULL;
}

void destoyTile(Tile * tile) {
	if (tile->living) {
		free(tile->living);
	}
	if (tile->zombies) {
		free(tile->zombies);
	}
}
