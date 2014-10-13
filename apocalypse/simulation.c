#include <stdbool.h>
#include <stdio.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "simulation.h"
#include "common.h"
#include "random.h"
#include "constants.h"
#include "log.h"
#include "direction.h"
#include "mpistuff.h"
#include "communication.h"

static int countNeighbouringZombies(WorldPtr world, int row, int column);
static EntityPtr findAdjacentFertileMale(WorldPtr world, int x, int y,
		simClock clock);
static bearing getBearing(WorldPtr world, int x, int y);
static void mergeStats(WorldPtr dest, Stats src);

/**
 * These macros require the worlds to be named input and output.
 * CAN_MOVE tests if the cell is empty in both worlds
 */
#define CAN_MOVE_TO(x, y, dir) \
	(GET_CELL_DIR((input), (dir), (x), (y)).type == NONE \
	&& GET_CELL_DIR((output), (dir), (x), (y)).type == NONE)

/**
 * IF_CAN_MOVE tests if the cell is empty in both worlds
 * and if it is, it returns the cell, otherwise it returns NULL
 */
#define IF_CAN_MOVE_TO(x, y, dir) \
	(CAN_MOVE_TO((x), (y), (dir)) ? &GET_CELL_DIR((output), (dir), (x), (y)) : NULL)

/**
 * Order of actions:
 * a) death of human or infected
 * b) decomposition of zombie
 * c) transition of infected to zombie
 * d) transition of human into infected
 */
static void simulateStep1(WorldPtr input, WorldPtr output) {
	simClock clock = output->clock;
// we want to force static scheduling because we suppose that the load
// is distributed evenly over the map and we need to have predictable locking
#ifdef _OPENMP
	// at least three columns per thread
	int threads = omp_get_max_threads();
	int numThreads = MIN(MAX(input->localWidth / 3, 1), threads);
#pragma omp parallel for num_threads(numThreads) schedule(static)
#endif
	for (int x = input->xStart; x < input->xEnd; x++) {
		Stats stats = NO_STATS;
		for (int y = input->yStart; y <= input->yEnd; y++) {
			EntityPtr entity = GET_CELL_PTR(input, x, y);
			if (entity->type == NONE) {
				continue;
			}

			// Death of living entity
			if (entity->type == HUMAN || entity->type == INFECTED) {
				if (randomDouble() < getDeathRate(entity, clock)) {
					if (entity->type == HUMAN) {
						if (entity->gender == FEMALE) {
							stats.humanFemalesDied++;
						} else {
							stats.humanMalesDied++;
						}
					} else {
						if (entity->gender == FEMALE) {
							stats.infectedFemalesDied++;
						} else {
							stats.infectedMalesDied++;
						}
					}
					LOG_EVENT("A %s died\n",
							entity->type == HUMAN ? "Human" : "Infected");
					// just forget this entity
					entity->type = NONE;
				}
			}

			// Decompose Zombie
			if (entity->type == ZOMBIE) {
				if (randomDouble() < getDecompositionRate(entity, clock)) {
					stats.zombiesDecomposed++;
					LOG_EVENT("A Zombie decomposed\n");
					// just forgot this entity
					entity->type = NONE;
				}
			}

			// Convert Infected to Zombie
			if (entity->type == INFECTED) {
				if (randomDouble() < PROBABILITY_BECOME_ZOMBIE) {
					if (entity->gender == FEMALE) {
						stats.infectedFemalesBecameZombies++;
					} else {
						stats.infectedMalesBecameZombies++;
					}
					toZombie(entity, clock);
					LOG_EVENT("An Infected became Zombie\n");
				}
			}
		}
		mergeStats(output, stats);
	}
}

/**
 * Order of actions:
 * a) transition of human into infected
 * b) giving birth to children - changes input
 * c) making love - changes input
 * d) movement
 */
static void simulateStep2(WorldPtr input, WorldPtr output) {
	simClock clock = output->clock;
	// notice that we iterate over xx and yy
	// and the real x and y are randomly switched between two directions
	double xxDir = randomDouble();
	double yyDir = randomDouble();

	// we want to force static scheduling because we suppose that the load
	// is distributed evenly over the map and we need to have predictable locking
#ifdef _OPENMP
	// at least three columns per thread
	int threads = omp_get_max_threads();
	int numThreads = MIN(MAX(input->localWidth / 3, 1), threads);
#pragma omp parallel for num_threads(numThreads) schedule(static)
#endif
	for (int xx = input->xStart; xx <= input->xEnd; xx++) {
		int x = (xxDir < 0.5) ? xx : (input->xEnd + input->xStart - xx);
		// stats are counted per column and summed at the end
		Stats stats = NO_STATS;
		lockColumn(output, x);
		for (int yy = input->yStart; yy <= input->yEnd; yy++) {
			int y = (yyDir < 0.5) ? yy : (input->yEnd + input->yStart - yy);
			Entity entity = GET_CELL(input, x, y);
			if (entity.type == NONE) {
				continue;
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
					LOG_EVENT("A Human became infected\n");
				}
			}

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

						Entity * freePtr;
						while (entity.children > 0 && (freePtr =
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
							*freePtr = child;
							LOG_EVENT("A %s child was born\n",
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
					EntityPtr adjacentMale = findAdjacentFertileMale(input, x,
							y, clock);
					if (adjacentMale != NULL) {
						stats.couplesMakingLove++;
						makeLove(&entity, adjacentMale, clock, input->stats);

						stats.childrenConceived += entity.children;
						LOG_EVENT("A couple made love\n");
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

			bearing bearing_ = getBearing(input, x, y); // optimal bearing
			bearing_ += getRandomBearing() * BEARING_FLUCTUATION;

			Direction dir = bearingToDirection(bearing_);
			if (dir != STAY) {
				double bearingRandomQuotient = (randomDouble() - 0.5)
						* BEARING_ABS_QUOTIENT_VARIANCE
						+ BEARING_ABS_QUOTIENT_MEAN;
				entity.bearing = bearing_ / cabsf(bearing_)
						* bearingRandomQuotient;
			} else {
				entity.bearing = bearing_;
			}

			// some randomness in direction
			// the entity will never go in the opposite direction
			if (dir != STAY) {
				if (randomDouble() < getMaxSpeed(&entity, clock)) {
					double dirRnd = randomDouble();
					if (dirRnd < DIRECTION_MISSED) {
						dir = DIRECTION_CCW(dir); // turn counter-clock-wise
					} else if (dirRnd < DIRECTION_MISSED * 2) {
						dir = DIRECTION_CW(dir); // turn clock-wise
					} else if (dirRnd
							> DIRECTION_FOLLOW + DIRECTION_MISSED * 2) {
						dir = STAY;
					}
				} else {
					dir = STAY;
				}
			} else {
				// if the entity would STAY, we'll try again to make it move
				// to make the entity bearing variable in terms of absolute value
				double bearingRandomQuotient = (randomDouble() - 0.5)
						* BEARING_ABS_QUOTIENT_VARIANCE
						+ BEARING_ABS_QUOTIENT_MEAN;

				bearing_ += getRandomBearing() * bearingRandomQuotient;
				dir = bearingToDirection(bearing_);
			}

			// we will try to find the cell in the chosen direction
			CellPtr destPtr = NULL;
			if (dir != STAY) {
				destPtr = IF_CAN_MOVE_TO(x, y, dir);
				if (destPtr == NULL) {
					destPtr = IF_CAN_MOVE_TO(x, y, DIRECTION_CCW(dir));
				}
				if (destPtr == NULL) {
					destPtr = IF_CAN_MOVE_TO(x, y, DIRECTION_CW(dir));
				}
			}
			if (destPtr == NULL) {
				destPtr = GET_CELL_PTR(output, x, y);
			}

			// actual assignment of entity to its destination
			*destPtr = entity;
		}
		unlockColumn(output, x);
		mergeStats(output, stats);
	}
}

void simulateStep(WorldPtr input, WorldPtr output) {
	output->clock = input->clock + 1;

	sendRecieveBorder(input);
	simulateStep1(input, output);
	sendRecieveBorderFinish(input);
	simulateStep2(input, output);
	sendReceiveGhosts(output);
	resetWorld(input);
	sendReceiveGhostsFinish(output);
}

/**
 * Returns a MALE Living entity which is on an adjacent cell to the given one.
 * The MALE has to be able to reproduce.
 */
static EntityPtr findAdjacentFertileMale(WorldPtr world, int x, int y,
		simClock clock) {
	int permutation = randomInt(0, RANDOM_BASIC_DIRECTIONS - 1);
	for (int i = 0; i < 4; i++) {
		Direction dir = random_basic_directions[permutation][i];
		EntityPtr entityPtr = GET_CELL_PTR_DIR(world, dir, x, y);
		if (entityPtr->type == HUMAN || entityPtr->type == INFECTED) {
			if (entityPtr->gender == MALE) {
				if (clock >= entityPtr->origin + entityPtr->fertilityStart
						&& clock
								< entityPtr->origin + entityPtr->fertilityEnd) {
					return entityPtr;
				}
			}
		}
	}
	return NULL;
}

/**
 * Returns the number of zombies in the cells bordering the cell at [x, y].
 */
static int countNeighbouringZombies(WorldPtr world, int x, int y) {
	int zombies = 0;
	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		if (GET_CELL_DIR(world, dir, x, y).type == ZOMBIE) {
			zombies++;
		}
	}
	return zombies;
}

/**
 * Returns the optimal bearing for an entity based on twelve adjacent cells:
 * __#__
 * _###_
 * ##@##
 * _###_
 * __#__
 * The cells in distance one and two are handled separately.
 * For each cell (and its entity) it is calculated the suitability to go that direction.
 * This may can produce a result which points to a cell which is occupied.
 * It is an intentional feature.
 */
static bearing getBearing(WorldPtr world, int x, int y) {
	Entity entity = GET_CELL(world, x, y);
	bearing bearing_ = entity.bearing;

	for (int dir = DIRECTION_START; dir <= DIRECTION_BASIC; dir++) {
		// all destinations are in the world
		CellPtr cellPtr = GET_CELL_PTR_DIR(world, dir, x, y);
		bearing delta = BEARING_FROM_DIRECTION(dir);

		if (entity.type == ZOMBIE) {
			if (IS_OUTSIDE(world, x + direction_delta_x[dir],
					y + direction_delta_y[dir])) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_WALL_ONE;
			} else if (cellPtr->type == NONE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_EMPTY_ONE;
			} else if (cellPtr->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_ZOMBIE_ONE;
			} else {
				bearing_ += delta * BEARING_RATE_ZOMBIE_LIVING_ONE;
			}
		} else {
			if (IS_OUTSIDE(world, x + direction_delta_x[dir],
					y + direction_delta_y[dir])) {
				bearing_ += delta * BEARING_RATE_LIVING_WALL_ONE;
			} else if (cellPtr->type == NONE) {
				bearing_ += delta * BEARING_RATE_LIVING_EMPTY_ONE;
			} else if (cellPtr->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_LIVING_ZOMBIE_ONE;
			} else if (cellPtr->gender != entity.gender) {
				bearing_ += delta * BEARING_RATE_LIVING_OPPOSITE_SEX_ONE;
			} else {
				bearing_ += delta * BEARING_RATE_LIVING_SAME_SEX_ONE;
			}
		}
	}
	for (int dir = DIRECTION_BASIC + 1; dir <= DIRECTION_ALL; dir++) {
		CellPtr cellPtr = GET_CELL_PTR_DIR(world, dir, x, y);
		bearing delta = BEARING_FROM_DIRECTION(dir);
		delta /= cabsf(delta);

		if (entity.type == ZOMBIE) {
			if (IS_OUTSIDE(world, x + direction_delta_x[dir],
					y + direction_delta_y[dir])) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_WALL_TWO;
			} else if (cellPtr->type == NONE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_EMPTY_TWO;
			} else if (cellPtr->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_ZOMBIE_ZOMBIE_TWO;
			} else {
				bearing_ += delta * BEARING_RATE_ZOMBIE_LIVING_TWO;
			}
		} else {
			if (IS_OUTSIDE(world, x + direction_delta_x[dir],
					y + direction_delta_y[dir])) {
				bearing_ += delta * BEARING_RATE_LIVING_WALL_TWO;
			} else if (cellPtr->type == NONE) {
				bearing_ += delta * BEARING_RATE_LIVING_EMPTY_TWO;
			} else if (cellPtr->type == ZOMBIE) {
				bearing_ += delta * BEARING_RATE_LIVING_ZOMBIE_TWO;
			} else if (cellPtr->gender != entity.gender) {
				bearing_ += delta * BEARING_RATE_LIVING_OPPOSITE_SEX_TWO;
			} else {
				bearing_ += delta * BEARING_RATE_LIVING_SAME_SEX_TWO;
			}
		}
	}

	return bearing_;
}

static void mergeStats(WorldPtr dest, Stats src) {
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
