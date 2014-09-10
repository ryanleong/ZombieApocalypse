#include <stdbool.h>
#include <stdio.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "simulation.h"
#include "common.h"
#include "random.h"
#include "constants.h"

void checkIncubationTime (Entity *infected, simClock currentTime);
void exposedToZombieNeighbours (World *world, int row, int column);
int countNeighbouringZombies (World *world, int row, int column);

double speedLimit(Entity * entity, simClock currentTime);

/**
 * These macros require the worlds to be named input and output
 */
#define CAN_MOVE_TO(x, y) \
	(GET_TILE((input), (x), (y))->entity == NULL \
	&& GET_TILE((output), (x), (y))->entity == NULL)

#define IF_CAN_MOVE_TO(x, y) \
	CAN_MOVE_TO(x, y) ? GET_TILE(output, x, y) : NULL

void simulateStep(World * input, World * output) {
	output->clock = input->clock + 1;

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

            // Infected people become zombies after a certain incubation
            // time, with some random variance.
            if (entity->type == INFECTED) {
                checkIncubationTime (entity, output->clock);
            }

			// Convert Human to Infected
			if (entity->type == HUMAN) {
                exposedToZombieNeighbours (input, y, x);
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
		Tile * in = GET_TILE(input, srcX, srcY); \
		if (in->entity == NULL) { \
			continue; \
		} \
		if (CAN_MOVE_TO(destX, destY)) { \
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

/**
 *  This function handles converting infected people to zombies, at the
 *  time specified when they were first infected.
 *
 *  First parameter is the entity, of type INFECTED, and the second param
 *  is the current time in the simulation.
 */
void checkIncubationTime (Entity *infected, simClock currentTime) {
    if (currentTime > infected->asInfected->becomesZombie) {
        infected = toZombie (infected, currentTime);
        infected->type = ZOMBIE;
        printf ("DEBUG: Incubation time ended.\n");
    }
}

/**
 *  This function checks all the neighbouring cells around a human and
 *  counts the number of zombies. If there are zombies, the human becomes
 *  infected, depending on a random number.
 */
void exposedToZombieNeighbours (World *world, int row, int column) {
    int zombieCount = countNeighbouringZombies (world, row, column);
    double infectionChance = zombieCount * PROBABILITY_INFECTION;
    Tile *tile = GET_TILE (world, column, row);
    Entity *entity = tile->entity;

    if(randomDouble() <= infectionChance) {
        entity = toInfected(entity->asHuman, world->clock);
        entity->type = INFECTED;
        printf ("DEBUG: New case in incubation.\n");
    }
}

/**
 *  Returns the number of zombies in the cells bordering the cell at 
 *  [row, column]. Indices outside the grid borders are ignored.
 */
int countNeighbouringZombies (World *world, int row, int column) {
    int zombies = 0;
    Tile *tile;
    Entity *entity;

    // step through all the cells of 1 row and 1 column either side of the
    // main cell.
    for (int i = row - 1; i <= row + 1; i ++) {
        for (int j = column - 1; j <= column + 1; j ++) {
            // are either of the indices outside the world boundaries? If so,
            // continue on until we get valid coords.
            if ((i < 0) || (i >= world->height) || (j < 0) || 
              (j >= world->width))
            {
                continue;
            }

            tile = GET_TILE (world, j, i);
            entity = tile->entity;

            // if the neighbouring cell is empty, move on to the next 
            // neighbour.
            if (entity == NULL)
                continue;

            if (entity->type == ZOMBIE)
                zombies ++;
        }
    }

    return zombies;
}

double speedLimit(Entity * entity, simClock currentTime) {
	double moveChance = 0.0;

	if(entity->type == ZOMBIE) {
		int zombieAge = (currentTime - entity->asZombie->becameZombie) / IN_YEARS;

		if(zombieAge < (ZOMBIE_DECOMPOZITION_MEAN / 2))
			moveChance = ZOMBIE_MOVE_SPEED_MEAN * 1.0;
		else
			moveChance = ZOMBIE_MOVE_SPEED_MEAN * 0.8;
	}
	else {
		int age = 0;
		if(entity->type == HUMAN) 
			age = (currentTime - entity->asHuman->wasBorn) / IN_YEARS;
		else 
			age = (currentTime - entity->asInfected->wasBorn) / IN_YEARS;

		moveChance = MALE_MOVE_SPEED_MEAN;

		if(age < 18)
			moveChance = moveChance * MALE_UNDER_18_SPEED_MEAN;
		else if(age > 39)
			moveChance = moveChance * MALE_OVER_40_SPEED_MEAN;

		if(entity->asHuman->gender == FEMALE)
			moveChance = moveChance * FEMALE_TO_MALE_SPEED_RATIO;
	}
	return moveChance;
}
