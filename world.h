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
#include "direction.h"

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

void lockTile(Tile * tile);

void unlockTile(Tile * tile);

typedef struct World {
	simClock clock;
	Tile ** map;
	unsigned int width;
	unsigned int height;
	unsigned int xStart;
	unsigned int xEnd;
	unsigned int yStart;
	unsigned int yEnd;
} World;

/*#define IN_WORLD(world, x, y) \
		((x) >= 0 && (x) <= (world)->width + 1 \
				&& (y) >= 0 && (y) <= (world)->height + 1)*/

#define GET_TILE(world, x, y) \
		((world)->map[(x)] + (y))

#define GET_TILE_DIR(world, dir, x, y) \
		GET_TILE((world), (x) + direction_delta_x[dir], (y) + direction_delta_y[dir])

World * newWorld(unsigned int width, unsigned int height);

void resetWorld(World * world);

void destoyWorld(World * world);

Tile * getFreeAdjacent(World * input, World * output, int x, int y);

#endif // WORLD_H_

// vim: ts=4 sw=4 et
