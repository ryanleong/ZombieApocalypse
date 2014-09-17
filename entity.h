/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include "clock.h"

/**
 *  This defines what entity is located at each cell in the grid. EMPTY
 *  means that there is no entity at the location.
 */
typedef enum {
    HUMAN, INFECTED, ZOMBIE, EMPTY
} entity_type_t;

typedef enum {
    MALE, FEMALE
} gender_t;


/**
 *  =============================================
 *  Structs for the three types of entities that are valid in the
 *  simulation. Humans can transition to infected when they are exposed
 *  to a zombie, and infected people become zombies at a certain rate.
 *  =============================================
 */

typedef struct {
    gender_t gender;
    sim_clock_t age;
} human_t;

typedef struct {
    gender_t gender;
    sim_clock_t became_infected;
} infected_t;

typedef struct {
    sim_clock_t became_zombie;
} zombie_t;


/**
 *  An entity in the simulation could be either human, infected or zombie,
 *  but only one of these at any given instant. Hence, a union is a logical
 *  choice of data structure for this problem. We will always use an
 *  entity_type_t to tell which field of this union is in use.
 */
typedef union {
    human_t human;
    infected_t infected;
    zombie_t zombie;
} entity_u;

/**
 *  These functions are used to seed the initial human and zombie
 *  populations.
 */
entity_u * new_human (void);
entity_u * new_zombie (void);

/**
 *  These functions handle transitions between different entity types.
 */
void becomes_infected (entity_u *human, sim_clock_t clock);
void becomes_zombie (entity_u *infected, sim_clock_t clock);

/**
 *  Functions for copying entities from one entity_u to another.
 */
void copy_human (human_t *origin, human_t *dest);
void copy_infected (infected_t *origin, infected_t *dest);
void copy_zombie (zombie_t *origin, zombie_t *dest);


#endif /* ENTITY_H_ */

// vim: ts=4 sw=4 et
