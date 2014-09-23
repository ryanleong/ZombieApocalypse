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
#include "stats.h"

/**
 * There are two types of Tiles:
 * REGULAR is inside the map where entities normally are
 * BORDER is a ghost tile which can host entities only temporarily
 */
typedef enum TileType {
	REGULAR, BORDER
} TileType;

/**
 * Each tile has its type: either REGULAR if it is inside or it is BORDER
 * Each tile also contains pointer to entity (which is NULL if the tile is empty)
 *
 */
typedef struct Tile {
	TileType type;
	Entity * entity;
} Tile;

/**
 * The World contains a 2D map.
 * The map is slightly bigger to allow unchecked access to two tiles in each direction.
 * You should never iterate like this: for (int x = 0; x < w->width; x++)
 * But rather: for (int x = w->xStart; x <= w->xEnd; x++)
 * Remember that start and end are inclusive.
 * The main reason is to make it possible to divide the map into pieces
 * which will calculated independently on different nodes connected by MPI.
 *
 * As there is never access more than 2 tiles from REGULAR tile,
 * no border checks are necessary.
 *
 * Part of the world (set of three adjacent columns) can be locked.
 */
typedef struct World {
	simClock clock;
	Tile ** map;
	unsigned int width; // real width
	unsigned int height; // real height
	unsigned int xStart; // first interior tile
	unsigned int xEnd; // last interior tile
	unsigned int yStart; // first interior tile
	unsigned int yEnd; // last interior tile
	Stats stats; // detailed statistics
	Stats lastStats;
#ifdef _OPENMP
	omp_lock_t * locks;
#endif
} World;

/**
 * Returns the specified tile.
 * Does not check if the tile is in the world.
 */
#define GET_TILE(world, x, y) \
		((world)->map[(x)] + (y))

/**
 * Returns the tile in direction from specified tile.
 * Does not check if the tile is in the world.
 */
#define GET_TILE_DIR(world, dir, x, y) \
		GET_TILE((world), (x) + direction_delta_x[(dir)], (y) + direction_delta_y[(dir)])

/**
 * Creates a new world of specified dimensions.
 */
World * newWorld(unsigned int width, unsigned int height);

/**
 * Resets the world - removes all entities.
 */
void resetWorld(World * world);

/**
 * Destroys the entities in the world and than destroys the world.
 */
void destroyWorld(World * world);

/**
 * Locks three adjacent columns centered around x for exclusive access.
 */
void lockColumn(World * world, int x);

/**
 * Unlocks the locked columns so everybody can use them.
 */
void unlockColumn(World * tile, int x);

/**
 * Returns the first adjacent tile to [x,y] in output which is free in both worlds.
 */
Tile * getFreeAdjacent(World * input, World * output, int x, int y);

void copyStats(World * world, Stats stats);

#endif // WORLD_H_

// vim: ts=4 sw=4 et
