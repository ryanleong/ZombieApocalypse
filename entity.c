#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

#include "entity.h"
#include "common.h"
#include "random.h"
#include "constants.h"
#include "utils.h"


/**
 *  Creates a new entity_u with the struct human field active. The age
 *  and gender will be chosen at random.
 */
    entity_u *
new_human (void)
{
    entity_u *entity = (entity_u *) checked_malloc (sizeof (entity_u));

    // generate a random int between 0 and 1 to decide the gender.
    entity->human.gender = (randomInt (0, 1) == 0)? MALE : FEMALE;

    // choose a random age between 0 and 75.
    entity->human.age = randomInt (0, 75);

    return entity;
}

/**
 *  This function is used to create an initial zombie population.
 */
    entity_u *
new_zombie (void)
{
    entity_u *entity = (entity_u *) checked_malloc (sizeof (entity_u));

    // initial zombie population will be assumed to have become zombies at
    // the start of the simulaion.
    entity->zombie.became_zombie = (sim_clock_t) 0;

    return entity;
}

/**
 *  Convert a struct human to struct infected. Note that the caller is
 *  responsible for ensuring that the entity_u passed to this function is
 *  in fact a human. This function cannot check.
 */
    void
becomes_infected (entity_u *entity, sim_clock_t now)
{
    gender_t gender = entity->human.gender;

    entity->infected.gender = gender;
    entity->infected.became_infected = now;
}

/**
 *  Converts a struct infected to a struct zombie. Same conventions apply
 *  as for becomes_infected.
 */
    void
becomes_zombie (entity_u *entity, sim_clock_t now)
{
    entity->zombie.became_zombie = now;
}

/**
 *  =============================================
 *  Functions for copying entities. Note that the entity struct that is
 *  copied into is overwritten, and also that the caller must check that
 *  the entity being copied is of the right type (HUMAN, INFECTED or 
 *  ZOMBIE).
 *  =============================================
 */

    void
copy_human (human_t *origin, human_t *dest)
{
    dest->gender = origin->gender;
    dest->age = origin->age;
}

    void
copy_infected (infected_t *origin, infected_t *dest)
{
    dest->gender = origin->gender;
    dest->became_infected = origin->became_infected;
}

    void
copy_zombie (zombie_t *origin, zombie_t *dest)
{
    dest->became_zombie = origin->became_zombie;
}

/** vim: set ts=4 sw=4 et : */
