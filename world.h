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


typedef struct {
	entity_type_t entity_type;
	entity_u entity;
#ifdef _OPENMP
	omp_lock_t lock;
#endif
} tile_t;

typedef struct {
	sim_clock_t clock;
	tile_t ** map;
	unsigned int width;
	unsigned int height;
} world_t;


world_t * newWorld(unsigned int width, unsigned int height);
void resetWorld(world_t * world);
void destoyWorld(world_t * world);
void lockTile(tile_t * tile);
void unlockTile(tile_t * tile);
bool valid_coordinates (const world_t *world, int row, int column);
int find_adjacent_space (const world_t *world, int *row, int *column);

#endif // WORLD_H_

// vim: ts=4 sw=4 et
