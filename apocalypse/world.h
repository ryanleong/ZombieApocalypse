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

typedef Entity Cell;
typedef EntityPtr CellPtr;

/**
 * The World contains a 2D map.
 * The map is slightly bigger to allow unchecked access to two cells in each direction.
 * You should never iterate like this: for (int x = 0; x < w->width; x++)
 * But rather: for (int x = w->xStart; x <= w->xEnd; x++)
 * Remember that start and end are inclusive.
 * The main reason is to make it possible to divide the map into pieces
 * which will calculated independently on different nodes connected by MPI.
 *
 * As there is never access more than 2 cells from internal cell,
 * no border checks are necessary.
 *
 * Part of the world (set of three adjacent columns) can be locked.
 */
typedef struct World {
	simClock clock;
	Cell ** map;
	unsigned int width; // real width
	unsigned int height; // real height
	unsigned int xStart; // first interior cell
	unsigned int xEnd; // last interior cell
	unsigned int yStart; // first interior cell
	unsigned int yEnd; // last interior cell
	Stats stats; // detailed statistics
	Stats lastStats;
#ifdef _OPENMP
omp_lock_t * locks;
#endif
} World;

typedef World * WorldPtr;

/**
 * Returns the specified entity.
 * Does not check if the entity is in the world.
 */
#define GET_CELL(worldPtr, x, y) \
		((worldPtr)->map[(x)][(y)])

#define GET_CELL_PTR(worldPtr, x, y) \
		(&GET_CELL((worldPtr), (x), (y)))

/**
 * Returns the cell in direction from specified cell.
 * Does not check if the cell is in the world.
 */
#define GET_CELL_DIR(worldPtr, dir, x, y) \
		GET_CELL((worldPtr), (x) + direction_delta_x[(dir)], (y) + direction_delta_y[(dir)])

#define GET_CELL_PTR_DIR(worldPtr, dir, x, y) \
		(&GET_CELL_DIR((worldPtr), (dir), (x), (y)))

/**
 * Tests if the specified field is outside the world.
 */
#define IS_OUTSIDE(worldPtr, x, y) \
	((x) < (worldPtr)->xStart || (y) < (worldPtr)->yStart \
	|| (x) > (worldPtr)->xEnd || (y) > (worldPtr)->yEnd)

/**
 * Creates a new world of specified dimensions.
 */
WorldPtr newWorld(unsigned int width, unsigned int height);

/**
 * Resets the world - removes all entities.
 */
void resetWorld(WorldPtr world);

/**
 * Destroys the entities in the world and than destroys the world.
 */
void destroyWorld(WorldPtr world);

/**
 * Locks three adjacent columns centered around x for exclusive access.
 */
void lockColumn(WorldPtr world, int x);

/**
 * Unlocks the locked columns so everybody can use them.
 */
void unlockColumn(WorldPtr world, int x);

/**
 * Returns the first adjacent cell to [x,y] in output which is free in both worlds.
 * Returns NULL if none is free.
 */
CellPtr getFreeAdjacent(WorldPtr input, WorldPtr output, int x, int y);

/**
 * Copies stats from earlier steps to this world
 */
void copyStats(WorldPtr world, Stats stats);

#endif // WORLD_H_

// vim: ts=4 sw=4 et
