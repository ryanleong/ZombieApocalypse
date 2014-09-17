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

int count_neighbouring_zombies (const world_t *world, int row, int column);
static bool adjacent_male (const world_t *world, int x, int y);

/** 
 *  Functions to convert entities to different entities, on certain 
 *  conditions including randomness. Each function takes two pointers to
 *  world_t structs, one for the state before the transition, the other
 *  is where we will store the new world state after the transition. Also
 *  there are the x and y coordinates of the subject entity.
 */
static bool infected_becomes_zombie (const world_t *before, world_t *after, 
  int x, int y);
static void human_reproduction (const world_t *before, world_t *after, 
  int x, int y);
static void human_death (const world_t *before, world_t *after, int x,
  int y);
static void infect_humans (const world_t *before, world_t *after, int x,
  int y);
static void zombie_decompose (const world_t *before, world_t *after, int x,
  int y);

// Function to move entities.
static void move_entity (const world_t *before, world_t *after, int x,
  int y);


/**
 *  Implements a single step of the simulation. This function will compute
 *  the next state of the world and store it in the world_t struct pointed
 *  to by the second param.
 */
    void
simulation_step (const world_t *input, world_t *output) 
{
    output->clock = input->clock + 1;

#ifdef _OPENMP
    int threads = omp_get_max_threads();
    int numThreads = MIN(MAX(input->width / 3, 1), threads);
    // at least three columns per thread
#pragma omp parallel for default(shared) num_threads(numThreads)
#endif
    for (int x = 0; x < input->width; x++) 
    {
        for (int y = 0; y < input->height; y++) 
        {
            switch (input->map [y] [x].entity_type)
            {
            case INFECTED:
                if (infected_becomes_zombie (input, output, x, y))
                    break;

                // fall through.

            case HUMAN:
                human_reproduction (input, output, x, y);
                human_death (input, output, x, y);
                break;

            case ZOMBIE:
                infect_humans (input, output, x, y);
                zombie_decompose (input, output, x, y);
                break;

            case EMPTY:
                // do nothing.
                break;
            }

            // humans (including infected) and zombies can all move.
            if (input->map [y] [x].entity_type != EMPTY)
                move_entity (input, output, x, y);
        }
    }
}

void finishStep(world_t * input, world_t * output) {
    resetWorld(input);
}

/**
 *  Search through adjacent cells to see if there are any males. Returns
 *  true if there is a male, false otherwise.
 */
    static bool
adjacent_male (const world_t *world, int x, int y)
{
    for (int row = y - 1; row <= y + 1; row ++)
    {
        for (int col = x - 1; col <= x + 1; col ++)
        {
            if (!valid_coordinates (world, row, col))
                continue;

            if (world->map [row] [col].entity_type == HUMAN)
            {
                if (world->map [row] [col].entity.human.gender == MALE)
                    return true;
            }
            else if (world->map [row] [col].entity_type == INFECTED)
            {
                if (world->map [row] [col].entity.infected.gender == MALE)
                    return true;
            }
        }
    }

    return false;
}

/**
 * Returns the number of zombies in the cells bordering the cell at [x, y].
 */
    int 
count_neighbouring_zombies (const world_t *world, int x, int y) 
{
    int zombies = 0;

    // step through all tiles within radius 1.
    for (int row = y - 1; row <= y + 1; row ++)
    {
        for (int col = x - 1; col <= x + 1; col ++)
        {
            // check that row,col is a valid grid coordinate.
            if (!valid_coordinates (world, row, col))
                continue;

            if (world->map [row] [col].entity_type == ZOMBIE)
                zombies ++;
        }
    }

    return zombies;
}

/** vim: set ts=4 sw=4 et : */
