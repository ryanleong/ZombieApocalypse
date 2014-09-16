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
#include "direction.h"

int countNeighbouringZombies(World *world, int row, int column);
LivingEntity * findAdjacentFertileMale(World * world, int x, int y,
		simClock clock);
bearing getBearing(World * world, int x, int y);

/**
 * These macros require the worlds to be named input and output
 */
#define CAN_MOVE_TO(x, y, dir) \
	(GET_TILE_DIR((input), (dir), (x), (y))->entity == NULL \
	&& GET_TILE_DIR((output), (dir), (x), (y))->entity == NULL)

#define IF_CAN_MOVE_TO(x, y, dir) \
	(CAN_MOVE_TO((x), (y), (dir)) ? GET_TILE_DIR((output), (dir), (x), (y)) : NULL)

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
	for (int x = input->xStart; x <= input->xEnd; x++) {
		for (int y = input->yStart; y <= input->yEnd; y++) {
			Entity * entity = GET_TILE(input, x, y)->entity;
			if (entity == NULL) {
				continue;
			}

			// Death of living entity
			if (entity->type == HUMAN || entity->type == INFECTED) {
				LivingEntity * le = entity->asLiving;
				if (le->willDie <= clock) {
					debug_printf("A %s died\n",
							entity->type == HUMAN ? "Human" : "Infected");
					continue; // just forget this entity
				}
			}

			// Decompose Zombie
			if (entity->type == ZOMBIE) {
				Zombie * zombie = entity->asZombie;
				if (zombie->decomposes <= clock) {
					debug_printf("A zombie decomposed\n");
					continue; // just forgot this entity
				}
				entity = copyEntity(entity);
			}

			// Convert Infected to Zombie
			if (entity->type == INFECTED) {
				Infected * infected = entity->asInfected;
				if (clock > infected->becomesZombie) {
					entity = toZombie(infected, clock)->asEntity;
					debug_printf("An infected became zombie\n");
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
					debug_printf("A human became infected\n");
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
						&& le->children.borns <= clock) {
					Tile * freeTile;
					while (le->children.count > 0 && (freeTile =
							getFreeAdjacent(input, output, x, y)) != NULL) {
						LivingEntity * child = giveBirth(le, clock);
						freeTile->entity = child->asEntity;
						debug_printf("A child was born\n");
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
						debug_printf("A couple made love\n");
					}
				}
			}

			// MOVEMENT

			bearing bearing = getBearing(input, x, y); // optimal
			bearing += getRandomBearing() * 0.35;
			entity->bearing = BEARING_PROJECT(bearing)
					* (randomDouble() * 0.5 + 0.75);

			Direction dir = bearingToDirection(bearing);

			// some randomness
			if (dir != STAY) {
				double dirRnd = randomDouble();
				if (dirRnd < DIRECTION_MISSED) {
					dir = (dir + 2) % 4 + 1; // turn counter-clock-wise
				} else if (dirRnd < DIRECTION_MISSED * 2) {
					dir = dir % 4 + 1; // turn clock-wise
				} else if (dirRnd > DIRECTION_FOLLOW + DIRECTION_MISSED * 2) {
					dir = STAY;
				}
			} else {
				bearing += getRandomBearing() * (randomDouble() * 0.5 + 0.75);
				dir = bearingToDirection(bearing);
			}

			Tile * dest = NULL;
			if (dir != STAY) {
				dest = IF_CAN_MOVE_TO(x, y, dir);
				if (dest == NULL) {
					dest = IF_CAN_MOVE_TO(x, y, DIRECTION_CCW(dir));
				}
				if (dest == NULL) {
					dest = IF_CAN_MOVE_TO(x, y, DIRECTION_CW(dir));
				}
			}
			if (dest == NULL) {
				dest = GET_TILE(output, x, y);
			}

			//lockTile(t);
			dest->entity = entity;
			//unlockTile(t);
		}
	}
}

#define MOVE_BACK(var, varMin, varMax, srcX, srcY, destX, destY) \
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
	MOVE_BACK(y, output->yStart, output->yEnd, y, output->xStart - 1, y,
			output->xStart)
	MOVE_BACK(y, output->yStart, output->yEnd, y, output->xEnd + 1, y,
			output->xEnd)
	MOVE_BACK(x, output->xStart, output->xEnd, output->yStart - 1, x,
			output->yStart, x)
	MOVE_BACK(x, output->xStart, output->xEnd, output->yEnd + 1, x,
			output->yEnd, x)

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
	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		if ((male = getFertileMale(GET_TILE_DIR(world, dir, x, y)->entity,
				clock)) != NULL) {
			return male;
		}
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
	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		if (isZombie(GET_TILE_DIR(world, dir, x, y)->entity)) {
			zombies++;
		}
	}
	return zombies;
}

bearing getBearing(World * world, int x, int y) {
	Entity * entity = GET_TILE(world, x, y)->entity;
	bearing bearing_ = entity->bearing;

	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		// all destinations are in the world
		Tile * t = GET_TILE_DIR(world, dir, x, y);
		bearing delta = BEARING_FROM_DIRECTION(dir);

		if (entity->type == ZOMBIE) {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_WALL_ONE;
			} else if (t->entity == NULL) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_EMPTY_ONE;
			} else if (t->entity->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_ZOMBIE_ONE;
			} else {
				bearing_ += delta * BEARING_RATE_ZOMBIE_LIVING_ONE;
			}
		} else {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_LIVING_WALL_ONE;
			} else if (t->entity == NULL) {
				bearing_ += delta * BEARING_RATE_LIVING_EMPTY_ONE;
			} else if (t->entity->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_LIVING_ZOMBIE_ONE;
			} else if (t->entity->asLiving->gender
					!= entity->asLiving->gender) {
				bearing_ += delta * BEARING_RATE_LIVING_OPPOSITE_SEX_ONE;
			} else {
				bearing_ += delta * BEARING_RATE_LIVING_SAME_SEX_ONE;
			}
		}
	}
	for (int dir = DIRECTION_BASIC + 1; dir <= DIRECTION_ALL; dir++) {
		Tile * t = GET_TILE_DIR(world, dir, x, y);
		bearing delta = BEARING_FROM_DIRECTION(dir);
		if (entity->type == ZOMBIE) {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_WALL_TWO;
			} else if (t->entity == NULL) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_EMPTY_TWO;
			} else if (t->entity->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_ZOMBIE_TWO;
			} else {
				bearing_ += delta * BEARING_RATE_ZOMBIE_LIVING_TWO;
			}
		} else {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_LIVING_WALL_TWO;
			} else if (t->entity == NULL) {
				bearing_ += delta * BEARING_RATE_LIVING_EMPTY_TWO;
			} else if (t->entity->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_LIVING_ZOMBIE_TWO;
			} else if (t->entity->asLiving->gender
					!= entity->asLiving->gender) {
				bearing_ += delta * BEARING_RATE_LIVING_OPPOSITE_SEX_TWO;
			} else {
				bearing_ += delta * BEARING_RATE_LIVING_SAME_SEX_TWO;
			}
		}
	}

	return bearing_;
}
