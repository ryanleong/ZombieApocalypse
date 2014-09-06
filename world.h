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

typedef enum TileType {
	REGULAR, BORDER
} TileType;

typedef struct Tile {
	TileType type;
	Entity * entity;
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
		((x) >= 0 && (x) <= (world)->width + 1 \
				&& (y) >= 0 && (y) <= (world)->height + 1)

#define GET_TILE(world, x, y) \
		((world)->map + (y) * ((world)->width + 2) + (x))

void initTile(Tile * tile);

void resetTile(Tile * tile);

void destroyTile(Tile * tile);

void lockTile(Tile * tile);

void unlockTile(Tile * tile);

#endif // WORLD_H_

// vim: ts=4 sw=4 et
