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
    sim_clock_t becameInfected;
} infected_t;

typedef struct {
    sim_clock_t becameZombie;
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
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new random human who lives prior to start of the simulation.
 * If the human is a female, she can be pregnant.
 */
human_t * newHuman(sim_clock_t clock);

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new zombie which will eventually decompose.
 */
zombie_t * newZombie(sim_clock_t clock);

/**
 * Converts a human into infected.
 * All attributes are preserved and becoming zombie is planned.
 * The human is disposed.
 */
infected_t * toInfected(human_t * human, sim_clock_t clock);

/**
 * Converts an infected into zombie.
 * The infected is disposed.
 */
zombie_t * toZombie(infected_t * infected, sim_clock_t clock);

/**
 * Copies human into a new entity, preserves all attributes.
 */
human_t * copyHuman(human_t * human);

/**
 * Copies infected into a new entity, preserves all attributes.
 */
infected_t * copyInfected(infected_t * infected);

/**
 * Copies zombie into a new entity, preserves all attributes.
 */
zombie_t * copyZombie(zombie_t * zombie);


#endif /* ENTITY_H_ */

// vim: ts=4 sw=4 et
