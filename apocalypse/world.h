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
#include "mpistuff.h"

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
	Cell ** map; // 2D array which is allocated as 1D
	Cell * map1d; // 1D array representation of the map
	Stats stats; // stats first for the local then for the global world

	unsigned int globalWidth; // real width
	unsigned int globalHeight; // real height
	unsigned int globalColumns;
	unsigned int globalRows;
	unsigned int globalX;
	unsigned int globalY;

	unsigned int localWidth; // real width of this world part
	unsigned int localHeight; // real height of this world part

	unsigned int xStart; // first interior cell
	unsigned int xEnd; // last interior cell
	unsigned int yStart; // first interior cell
	unsigned int yEnd; // last interior cell

#ifdef _OPENMP
omp_lock_t * locks;
#endif
#ifdef USE_MPI
MPI_Comm comm;
MPI_Request requests[MAX_REQUESTS];
int requestCount;
MPI_Datatype rowType;
MPI_Datatype columnType;
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
 * Creates a new world of specified dimensions as it is the only one.
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

#endif // WORLD_H_

// vim: ts=4 sw=4 et
