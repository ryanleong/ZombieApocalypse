#include <stdbool.h>
#include <stdio.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "simulation.h"
#include "common.h"
#include "random.h"
#include "constants.h"
#include "debug.h"

int countNeighbouringZombies(World *world, int row, int column);
LivingEntity * findAdjacentFertileMale(World * world, int x, int y,
		simClock clock);

/**
 * These macros require the worlds to be named input and output
 */
#define CAN_MOVE_TO(x, y) \
	(GET_TILE((input), (x), (y))->entity == NULL \
	&& GET_TILE((output), (x), (y))->entity == NULL)

#define IF_CAN_MOVE_TO(x, y) \
	CAN_MOVE_TO(x, y) ? GET_TILE(output, x, y) : NULL

/**
 * Order of actions:
 * a) death of human or infected
 * b) decomposition of zombie
 * c) transition of infected to zombie
 * d) transition of human into infected
 * e) giving birth to children - changes input
 * f) making love - changes input
 * g) movement
 */
void simulateStep(World * input, World * output) {
	simClock clock = output->clock = input->clock + 1;

#ifdef _OPENMP
	int threads = omp_get_max_threads();
	int numThreads = MIN(MAX(input->width / 3, 1), threads);
	// at least three columns per thread
#pragma omp parallel for default(shared) num_threads(numThreads)
#endif
	for (int x = 1; x <= input->width; x++) {
		for (int y = 1; y <= input->height; y++) {
			Entity * entity = GET_TILE(input, x, y)->entity;
			if (entity == NULL) {
				continue;
			}

			// Death of living entity
			if (entity->type == HUMAN || entity->type == INFECTED) {
				LivingEntity * le = entity->asLiving;
				if (le->willDie <= clock) {
					dprintf("A %s died\n",
							entity->type == HUMAN? "Human" : "Infected");
					continue; // just forget this entity
				}
			}

			// Decompose Zombie
			if (entity->type == ZOMBIE) {
				Zombie * zombie = entity->asZombie;
				if (zombie->decomposes <= clock) {
					dprintf("A zombie decomposed\n");
					continue; // just forgot this entity
				}
				entity = copyEntity(entity);
			}

			// Convert Infected to Zombie
			if (entity->type == INFECTED) {
				Infected * infected = entity->asInfected;
				if (clock > infected->becomesZombie) {
					entity = toZombie(infected, clock)->asEntity;
					dprintf("An infected became zombie\n");
				} else {
					entity = copyEntity(entity);
				}
			}

			// Convert Human to Infected
			if (entity->type == HUMAN) {
				int zombieCount = countNeighbouringZombies(input, x, y);
				double infectionChance = zombieCount * PROBABILITY_INFECTION;

				if (randomDouble() <= infectionChance) {
					entity = toInfected(entity->asHuman, clock)->asEntity;
					dprintf("A human became infected\n");
				} else {
					entity = copyEntity(entity);
				}
			}

			// Here the variable entity contains either copy or a new entity after transition.
			// This is important. From now on, we may freely change it.

			// Here are performed natural processed
			if (entity->type == HUMAN || entity->type == INFECTED) {
				LivingEntity * le = entity->asLiving;
				// giving birth
				if (le->gender == FEMALE && le->children.count > 0
						&& le->children.borns >= clock) {
					Tile * freeTile;
					while (le->children.count > 0 && (freeTile =
							getFreeAdjacent(input, output, x, y)) != NULL) {
						LivingEntity * child = giveBirth(le, clock);
						freeTile->entity = child->asEntity;
						dprintf("A child was born\n");
					}
				}

				// making love
				if (le->gender == FEMALE && le->children.count == 0
						&& clock >= le->fertilityStart
						&& clock < le->fertilityEnd) { // can have baby
					LivingEntity *adjacentMale = findAdjacentFertileMale(input,
							x, y, clock);
					if (adjacentMale != NULL) {
						makeLove(le, adjacentMale, clock);
						dprintf("A couple made love\n");
					}
				}
			}

			// just an example of random movement

			int rnd = randomInt(0, 4);
			Tile * t = NULL;
			if (rnd != 0) {
				switch (rnd) {
				case 1:
					t = IF_CAN_MOVE_TO(x + 1, y);
					break;
				case 2:
					t = IF_CAN_MOVE_TO(x, y + 1);
					break;
				case 3:
					t = IF_CAN_MOVE_TO(x - 1, y);
					break;
				case 4:
					t = IF_CAN_MOVE_TO(x, y - 1);
					break;
				}
			}
			if (t == NULL) {
				t = GET_TILE(output, x, y);
			}
			lockTile(t);
			Entity * e = copyEntity(entity);
			t->entity = e;
			unlockTile(t);
		}
	}
}

#define MOVE_BACK(var, varMax, srcX, srcY, destX, destY) \
	for (int var = 0; var < varMax; var++) { \
		Tile * in = GET_TILE(output, srcX, srcY); \
		if (in->entity == NULL) { \
			continue; \
		} \
		if (GET_TILE(output, destX, destY)->entity == NULL) { \
			GET_TILE(output, destX, destY)->entity = in->entity; \
		} \
		in->entity = NULL; \
	}

void finishStep(World * input, World * output) {
	MOVE_BACK(x, output->width, x, 0, x, 1)
	MOVE_BACK(x, output->width, x, output->height + 1, x, output->height)
	MOVE_BACK(y, output->width, 0, y, 1, y)
	MOVE_BACK(y, output->width, output->width + 1, y, output->width, y)

	resetWorld(input);
}

LivingEntity * findAdjacentFertileMale(World * world, int x, int y,
		simClock clock) {
	LivingEntity * getFertileMale(Entity * entity, simClock clock) {
		if (entity == NULL) {
			return NULL;
		}

		if (entity->type == HUMAN || entity->type == INFECTED) {
			LivingEntity * le = entity->asLiving;
			if (le->gender == MALE) {
				if (clock >= le->fertilityStart && clock < le->fertilityEnd) {
					return le;
				}
			}
		}
		return NULL;
	}

	LivingEntity * male;
	if ((male = getFertileMale(GET_TILE_LEFT(world, x, y)->entity, clock))
			!= NULL) {
		return male;
	}
	if ((male = getFertileMale(GET_TILE_UP(world, x, y)->entity, clock)) != NULL) {
		return male;
	}
	if ((male = getFertileMale(GET_TILE_RIGHT(world, x, y)->entity, clock))
			!= NULL) {
		return male;
	}
	if ((male = getFertileMale(GET_TILE_DOWN(world, x, y)->entity, clock))
			!= NULL) {
		return male;
	}
	return NULL;
}

/**
 * Returns the number of zombies in the cells bordering the cell at [x, y].
 */
int countNeighbouringZombies(World * world, int x, int y) {
	bool isZombie(Entity * entity) {
		return entity != NULL && entity->type == ZOMBIE;
	}

	int zombies = 0;
	if (isZombie(GET_TILE_LEFT(world, x, y)->entity)) {
		zombies++;
	}
	if (isZombie(GET_TILE_UP(world, x, y)->entity)) {
		zombies++;
	}
	if (isZombie(GET_TILE_RIGHT(world, x, y)->entity)) {
		zombies++;
	}
	if (isZombie(GET_TILE_DOWN(world, x, y)->entity)) {
		zombies++;
	}

	return zombies;
}
