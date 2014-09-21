/**
 *  Defines a structure for each cell of the grid over which we are
 *  simulating a zombie outbreak.
 */

#ifndef WORLD_H_
#define WORLD_H_

#include <stdlib.h>
#include <stdbool.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "clock.h"
#include "entity.h"
#include "direction.h"


typedef struct {
	entity_type_t entity_type;
	entity_u entity;
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
typedef struct {
	sim_clock_t clock;
	Tile ** map;
	unsigned int width;
	unsigned int height;
	unsigned int xStart; // first interior tile
	unsigned int xEnd; // last interior tile
	unsigned int yStart; // first interior tile
	unsigned int yEnd; // last interior tile
#ifdef _OPENMP
    omp_lock_t * locks;
#endif
} World;


World * newWorld(unsigned int width, unsigned int height);
void resetWorld(World * world);
void destoyWorld(World * world);
void lockTile(Tile * tile);
void unlockTile(Tile * tile);
bool valid_coordinates (const World *world, int y, int x);
int find_adjacent_space (const World *world, int *row, int *column);


#endif // WORLD_H_

// vim: ts=4 sw=4 et
