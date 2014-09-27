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

static int countNeighbouringZombies(World *world, int row, int column);
static Entity * findAdjacentFertileMale(World * world, int x, int y,
		simClock clock);
static bearing getBearing(World * world, int x, int y);
static void mergeStats(World * dest, Stats src);

/**
 * These macros require the worlds to be named input and output.
 * CAN_MOVE tests if the tile is empty in both worlds
 */
#define CAN_MOVE_TO(x, y, dir) \
	(GET_TILE_DIR((input), (dir), (x), (y))->entity.type == NONE \
	&& GET_TILE_DIR((output), (dir), (x), (y))->entity.type == NONE)

/**
 * IF_CAN_MOVE tests if the tile is empty in both worlds
 * and if it is, it returns the tile, otherwise it returns NULL
 */
#define IF_CAN_MOVE_TO(x, y, dir) \
	(CAN_MOVE_TO((x), (y), (dir)) ? GET_TILE_DIR((output), (dir), (x), (y)) : NULL)

/**
 * Note that this function is called by every thread.
 *
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

	// we want to force static scheduling because we suppose that the load
	// is distributed evenly over the map
#ifdef _OPENMP
#pragma omp parallel for num_threads(getNumThreads(input->width)) schedule(static)
#endif
	for (int x = input->xStart; x <= input->xEnd; x++) {
		// stats are counted per column and summed at the end
		Stats stats = NO_STATS;
		lockColumn(output, x);
		for (int y = input->yStart; y <= input->yEnd; y++) {
			Entity entity = GET_TILE(input, x, y)->entity;
			if (entity.type == NONE) {
				continue;
			}

			// Death of living entity
			if (entity.type == HUMAN || entity.type == INFECTED) {
				if (randomDouble() < getDeathRate(&entity, clock)) {
					if (entity.type == HUMAN) {
						if (entity.gender == FEMALE) {
							stats.humanFemalesDied++;
						} else {
							stats.humanMalesDied++;
						}
					} else {
						if (entity.gender == FEMALE) {
							stats.infectedFemalesDied++;
						} else {
							stats.infectedMalesDied++;
						}
					}
					debug_printf("A %s died\n",
							entity.type == HUMAN ? "Human" : "Infected");
					continue; // just forget this entity
				}
			}

			// Decompose Zombie
			if (entity.type == ZOMBIE) {
				if (randomDouble() < getDecompositionRate(&entity, clock)) {
					stats.zombiesDecomposed++;
					debug_printf("A Zombie decomposed\n");
					continue; // just forgot this entity
				}
			}

			// Convert Infected to Zombie
			if (entity.type == INFECTED) {
				if (randomDouble() < PROBABILITY_BECOME_ZOMBIE) {
					if (entity.gender == FEMALE) {
						stats.infectedFemalesBecameZombies++;
					} else {
						stats.infectedMalesBecameZombies++;
					}
					toZombie(&entity, clock);
					debug_printf("An Infected became Zombie\n");
				}
			}

			// Convert Human to Infected
			if (entity.type == HUMAN) {
				int zombieCount = countNeighbouringZombies(input, x, y);
				double infectionChance = zombieCount * PROBABILITY_INFECTION;

				if (randomDouble() <= infectionChance) {
					if (entity.gender == FEMALE) {
						stats.humanFemalesBecameInfected++;
					} else {
						stats.humanMalesBecameInfected++;
					}
					toInfected(&entity, clock);
					debug_printf("A Human became infected\n");
				}
			}

			// Here the entity variable contains either copy or a new entity after transition.
			// This is important. From now on, we may freely change it.

			// Here are performed natural processed of humans and infected
			if (entity.type == HUMAN || entity.type == INFECTED) {
				// giving birth
				if (entity.gender == FEMALE && entity.children > 0) {
					if (entity.origin + entity.borns <= clock) {
						if (entity.type == HUMAN) {
							stats.humanFemalesGivingBirth++;
						} else {
							stats.infectedFemalesGivingBirth++;
						}

						Tile * freeTile;
						while (entity.children > 0 && (freeTile =
								getFreeAdjacent(input, output, x, y)) != NULL) {
							Entity child = giveBirth(&entity, clock);
							if (child.type == HUMAN) {
								if (child.gender == FEMALE) {
									stats.humanFemalesBorn++;
								} else {
									stats.humanMalesBorn++;
								}
							} else {
								if (child.gender == FEMALE) {
									stats.infectedFemalesBorn++;
								} else {
									stats.infectedMalesBorn++;
								}
							}
							freeTile->entity = child;
							debug_printf("A %s child was born\n",
									child.type == HUMAN ? "Human" : "Infected");
						}
					} else {
						if (entity.type == HUMAN) {
							stats.humanFemalesPregnant++;
						} else {
							stats.infectedFemalesPregnant++;
						}
					}
				}

				// making love
				if (entity.gender == FEMALE && entity.children == 0
						&& clock >= entity.origin + entity.fertilityStart
						&& clock < entity.origin + entity.fertilityEnd) { // can have baby
					Entity * adjacentMale = findAdjacentFertileMale(input, x, y,
							clock);
					if (adjacentMale != NULL) {
						stats.couplesMakingLove++;
						makeLove(&entity, adjacentMale, clock,
								input->lastStats);

						stats.childrenConceived += entity.children;
						debug_printf("A couple made love\n");
					}
				}
			}

			if (entity.type == HUMAN) {
				if (entity.gender == FEMALE) {
					stats.humanFemales++;
				} else {
					stats.humanMales++;
				}
			} else if (entity.type == INFECTED) {
				if (entity.gender == FEMALE) {
					stats.infectedFemales++;
				} else {
					stats.infectedMales++;
				}
			} else {
				stats.zombies++;
			}

			// MOVEMENT

			bearing bearing = getBearing(input, x, y); // optimal bearing
			bearing += getRandomBearing() * BEARING_FLUCTUATION;

			// to make the entity bearing variable in terms of absolute value
			double bearingRandomQuotient = (randomDouble() - 0.5)
					* BEARING_ABS_QUOTIENT_VARIANCE + BEARING_ABS_QUOTIENT_MEAN;
			entity.bearing = BEARING_PROJECT(bearing) * bearingRandomQuotient;

			Direction dir = bearingToDirection(bearing);

			// some randomness in direction
			// the entity will never go in the opposite direction
			if (dir != STAY) {
				if (randomDouble() < getMaxSpeed(&entity, clock)) {
					double dirRnd = randomDouble();
					if (dirRnd < DIRECTION_MISSED) {
						dir = (dir + 2) % 4 + 1; // turn counter-clock-wise
					} else if (dirRnd < DIRECTION_MISSED * 2) {
						dir = dir % 4 + 1; // turn clock-wise
					} else if (dirRnd
							> DIRECTION_FOLLOW + DIRECTION_MISSED * 2) {
						dir = STAY;
					}
				} else {
					dir = STAY;
				}
			} else {
				// if the entity would STAY, we'll try again to make it move
				// TODO this should be reviewed; does it make sense?
				bearing += getRandomBearing() * (randomDouble() * 0.5 + 0.75);
				dir = bearingToDirection(bearing);
			}

			// we will try to find the tile in the chosen direction
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

			// actual assignment of entity to its destination
			dest->entity = entity;
		}
		unlockColumn(output, x);
		mergeStats(output, stats);
	}
}

/**
 * Moves back an entity which is on the BORDER tile.
 */
void moveBack(World * world, int srcX, int srcY, int destX, int destY) {
	Tile * in = GET_TILE(world, srcX, srcY);
	if (in->entity.type != NONE) {
		if (GET_TILE(world, destX, destY)->entity.type == NONE) {
			GET_TILE(world, destX, destY)->entity = in->entity;
		}
		in->entity.type = NONE;
	}
}

void finishStep(World * input, World * output) {
	{
#ifdef _OPENMP
		int threads = omp_get_max_threads();
		int numThreads = MIN(MAX(output->width / 10, 1), threads);
#pragma omp parallel for schedule(guided, 10) num_threads(numThreads)
#endif
		for (int x = output->xStart; x <= output->xEnd; x++) {
			moveBack(output, x, output->yStart - 1, x, output->yStart);
			moveBack(output, x, output->yEnd + 1, x, output->yEnd);
		}
	}

	{
#ifdef _OPENMP
		int threads = omp_get_max_threads();
		int numThreads = MIN(MAX(output->height / 10, 1), threads);
#pragma omp parallel for schedule(guided, 10) num_threads(numThreads)
#endif
		for (int y = output->yStart; y <= output->yEnd; y++) {
			moveBack(output, output->xStart - 1, y, output->xStart, y);
			moveBack(output, output->xEnd + 1, y, output->xEnd, y);
		}
	}
}

/**
 * Returns a MALE Living entity which is on an adjacent tile to the given one.
 * The MALE has to be able to reproduce.
 */
static Entity * findAdjacentFertileMale(World * world, int x, int y,
		simClock clock) {
	Entity * getFertileMale(Entity * entity, simClock clock) {
		if (entity->type == HUMAN || entity->type == INFECTED) {
			if (entity->gender == MALE) {
				if (clock >= entity->origin + entity->fertilityStart
						&& clock < entity->origin + entity->fertilityEnd) {
					return entity;
				}
			}
		}
		return NULL;
	}

	Entity * male;
	int permutation = randomInt(0, RANDOM_BASIC_DIRECTIONS - 1);
	for (int i = 0; i < 4; i++) {
		Direction dir = random_basic_directions[permutation][i];
		if ((male = getFertileMale(&GET_TILE_DIR(world, dir, x, y)->entity,
				clock)) != NULL) {
			return male;
		}
	}
	return NULL;
}

/**
 * Returns the number of zombies in the cells bordering the cell at [x, y].
 */
static int countNeighbouringZombies(World * world, int x, int y) {
	int zombies = 0;
	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		if (GET_TILE_DIR(world, dir, x, y)->entity.type == ZOMBIE) {
			zombies++;
		}
	}
	return zombies;
}

/**
 * Returns the optimal bearing for an entity based on twelve adjacent tiles:
 * __#__
 * _###_
 * ##@##
 * _###_
 * __#__
 * The tiles in distance one and two are handled separately.
 * For each tile (and its entity) it is calculated the suitability to go that direction.
 * This may can produce a result which points to a tile which is occupied.
 * It is an intentional feature.
 */
static bearing getBearing(World * world, int x, int y) {
	Entity entity = GET_TILE(world, x, y)->entity;
	bearing bearing_ = entity.bearing;

	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		// all destinations are in the world
		Tile * t = GET_TILE_DIR(world, dir, x, y);
		bearing delta = BEARING_FROM_DIRECTION(dir);

		if (entity.type == ZOMBIE) {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_WALL_ONE;
			} else if (t->entity.type == NONE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_EMPTY_ONE;
			} else if (t->entity.type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_ZOMBIE_ONE;
			} else {
				bearing_ += delta * BEARING_RATE_ZOMBIE_LIVING_ONE;
			}
		} else {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_LIVING_WALL_ONE;
			} else if (t->entity.type == NONE) {
				bearing_ += delta * BEARING_RATE_LIVING_EMPTY_ONE;
			} else if (t->entity.type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_LIVING_ZOMBIE_ONE;
			} else if (t->entity.gender != entity.gender) {
				bearing_ += delta * BEARING_RATE_LIVING_OPPOSITE_SEX_ONE;
			} else {
				bearing_ += delta * BEARING_RATE_LIVING_SAME_SEX_ONE;
			}
		}
	}
	for (int dir = DIRECTION_BASIC + 1; dir <= DIRECTION_ALL; dir++) {
		Tile * t = GET_TILE_DIR(world, dir, x, y);
		bearing delta = BEARING_PROJECT(BEARING_FROM_DIRECTION(dir));
		if (entity.type == ZOMBIE) {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_WALL_TWO;
			} else if (t->entity.type == NONE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_EMPTY_TWO;
			} else if (t->entity.type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_ZOMBIE_TWO;
			} else {
				bearing_ += delta * BEARING_RATE_ZOMBIE_LIVING_TWO;
			}
		} else {
			if (t->type == BORDER) {
				bearing_ += delta * BEARING_RATE_LIVING_WALL_TWO;
			} else if (t->entity.type == NONE) {
				bearing_ += delta * BEARING_RATE_LIVING_EMPTY_TWO;
			} else if (t->entity.type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_LIVING_ZOMBIE_TWO;
			} else if (t->entity.gender != entity.gender) {
				bearing_ += delta * BEARING_RATE_LIVING_OPPOSITE_SEX_TWO;
			} else {
				bearing_ += delta * BEARING_RATE_LIVING_SAME_SEX_TWO;
			}
		}
	}

	return bearing_;
}

static void mergeStats(World * dest, Stats src) {
#ifdef _OPENMP
#pragma omp critical (StatsCriticalRegion)
#endif
	{
		dest->stats.humanFemales += src.humanFemales;
		dest->stats.humanMales += src.humanMales;
		dest->stats.infectedFemales += src.infectedFemales;
		dest->stats.infectedMales += src.infectedMales;
		dest->stats.zombies += src.zombies;
		dest->stats.humanFemalesDied += src.humanFemalesDied;
		dest->stats.humanMalesDied += src.humanMalesDied;
		dest->stats.infectedFemalesDied += src.infectedFemalesDied;
		dest->stats.infectedMalesDied += src.infectedMalesDied;
		dest->stats.zombiesDecomposed += src.zombiesDecomposed;
		dest->stats.humanFemalesBorn += src.humanFemalesBorn;
		dest->stats.humanMalesBorn += src.humanMalesBorn;
		dest->stats.humanFemalesGivingBirth += src.humanFemalesGivingBirth;
		dest->stats.humanFemalesPregnant += src.humanFemalesPregnant;
		dest->stats.infectedFemalesBorn += src.infectedFemalesBorn;
		dest->stats.infectedMalesBorn += src.infectedMalesBorn;
		dest->stats.infectedFemalesGivingBirth +=
				src.infectedFemalesGivingBirth;
		dest->stats.infectedFemalesPregnant += src.infectedFemalesPregnant;
		dest->stats.couplesMakingLove += src.couplesMakingLove;
		dest->stats.childrenConceived += src.childrenConceived;
		dest->stats.humanFemalesBecameInfected +=
				src.humanFemalesBecameInfected;
		dest->stats.humanMalesBecameInfected += src.humanMalesBecameInfected;
		dest->stats.infectedFemalesBecameZombies +=
				src.infectedFemalesBecameZombies;
		dest->stats.infectedMalesBecameZombies +=
				src.infectedMalesBecameZombies;
	}
}

int getNumThreads(int width) {
#ifdef _OPENMP
// at least three columns per thread
	int threads = omp_get_max_threads();
	int numThreads = MIN(MAX(width / 3, 1), threads);
#else
	int numThreads = 1;
#endif
	return numThreads;
}
