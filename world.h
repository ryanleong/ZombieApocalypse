/**
 *  Defines a structure for each cell of the grid over which we are
 *  simulating a zombie outbreak.
 */

#ifndef WORLD_H_
#define WORLD_H_

#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "clock.h"
#include "entity.h"

typedef struct Tile {
	// linked lists to represent all the humans and zombies in a particular tile
	LivingEntity * living;
	Zombie * zombies;
#ifdef _OPENMP
	omp_lock_t lock;
#endif
} Tile;

typedef struct World {
	simClock clock;
	Tile * map;
	unsigned int width;
	unsigned int height;
} World;

World * newWorld(unsigned int width, unsigned int height);

void resetWorld(World * world);

void destoyWorld(World * world);

#define IN_WORLD(world, x, y) \
		((x) >= 0 && (x) < (world)->width && (y) >= 0 && (y) < (world)->height)

/* we definitely want to inline this, therefore it is in header */
static Tile * getTile(World * w, int x, int y) {
	if (!IN_WORLD(w, x, y)) {
		return NULL;
	}
	return w->map + y * w->width + x;
}

void initTile(Tile * tile);

void resetTile(Tile * tile);

void destroyTile(Tile * tile);

void lockTile(Tile * tile);

void unlockTile(Tile * tile);

#endif // WORLD_H_

// vim: ts=4 sw=4 et
