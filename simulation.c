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

	// DEBUG
	int zCount = 0, hCount = 0, iCount = 0;

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
				int zombieCount = 0;

				Entity * tempEntity = GET_TILE(input, x + 1, y)->entity;
				if (tempEntity != NULL && tempEntity->type == ZOMBIE) {
					zombieCount++;
				}

				tempEntity = GET_TILE(input, x - 1, y)->entity;
				if (tempEntity != NULL && tempEntity->type == ZOMBIE) {
					zombieCount++;
				}

				tempEntity = GET_TILE(input, x, y + 1)->entity;
				if (tempEntity != NULL && tempEntity->type == ZOMBIE) {
					zombieCount++;
				}

				tempEntity = GET_TILE(input, x, y + 1)->entity;
				if (tempEntity != NULL && tempEntity->type == ZOMBIE) {
					zombieCount++;
				}

				// Convert to infected
				double infectionChance = zombieCount * PROBABILITY_INFECTION;
				if(randomDouble() <= infectionChance) {
					entity = toInfected(entity->asHuman, output->clock);
					entity->type = INFECTED;
                    printf ("DEBUG: New case in incubation.\n");
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

			// TODO continue here by specifying rules

			// DEBUG
			if(entity->type == HUMAN) {
				hCount++;
			}
			else if(entity->type == INFECTED) {
				iCount++;
			}
			else if(entity->type == ZOMBIE) {
				zCount++;
			}
		}
	}
	printf("Humans: %d, Infected: %d, Zombie: %d\n", hCount, iCount, zCount);
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

void checkIncubationTime (Entity *infected, simClock currentTime) {
    if (currentTime > infected->asInfected->becomesZombie) {
        infected = toZombie (infected, currentTime);
        infected->type = ZOMBIE;
        printf ("DEBUG: Incubation time ended.\n");
    }
}
