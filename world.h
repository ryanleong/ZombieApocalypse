/**
 *  Defines a structure for each cell of the grid over which we are
 *  simulating a zombie outbreak.
 */

#ifndef WORLD_H_
#define WORLD_H_

#include <stdlib.h>

#include "clock.h"
#include "agent.h"

typedef struct Tile {
	// linked lists to represent all the humans and zombies in a particular tile
	LivingEntity * living;
	Zombie * zombies;
} Tile;

typedef struct World {
	simClock clock;
	Tile ** map;
	unsigned int width;
	unsigned int height;
} World;

World * newWorld(unsigned int width, unsigned int height);

void destoyWorld(World * world);

void initTile(Tile * tile);

void resetTile(Tile * tile);

void destoyTile(Tile * tile);

#endif // WORLD_H_

// vim: ts=4 sw=4 et
