/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include "clock.h"
#include "direction.h"
#include "stats.h"

/**
 * Entity may be of type HUMAN, INFECTED or ZOMBIE.
 * Each entity has its own attributes but some of them are shared.
 * HUMAN and INFECTED are together a LivingEntity.
 */
typedef enum EntityType {
	HUMAN, INFECTED, ZOMBIE
} EntityType;

/**
 * An Entity is an agent in the World
 */
typedef struct Entity Entity;

/**
 * Living entity is an Entity.
 * It has gender, wasBorn and information about fertility and children.
 */
typedef struct LivingEntity LivingEntity;

/**
 * Human is a basic Living entity.
 */
typedef struct Human Human;

/**
 * Infected is a Living entity with information about infection.
 */
typedef struct Infected Infected;

/**
 * Zombie is not a Living entity; it does not have gender nor other attributes.
 * Zombie has a date when it was created.
 */
typedef struct Zombie Zombie;

/**
 * Children is a structure which is inside Living entities.
 */
typedef struct Children {
	int count;
	simClock conceived;
	simClock borns;
} Children;

/**
 * Convenient macro for resetting children information.
 */
#define NO_CHILDREN ((const struct Children){0, 0, 0})

/*
 * First field is an enum specifying type of entity.
 * Second field is a pointer to itself to simplify "subclass" access.
 *
 * We massively use prefix property of structs - common prefix of two structs
 * is structuralized identically - located at the same offset in the struct.
 */
#define ENTITY_FIELDS \
	EntityType type; \
	union { \
		Entity * asEntity; \
		LivingEntity * asLiving; \
		Human * asHuman; \
		Infected * asInfected; \
		Zombie * asZombie; \
	}; \
	bearing bearing; \
	// put general entity properties here

/**
 * We have only two genders - MALE and FEMALE.
 * Sorry http://en.wikipedia.org/wiki/Third_gender
 */
typedef enum Gender {
	MALE, FEMALE
} Gender;

struct Entity {
	ENTITY_FIELDS
};

#define LIVING_ENTITY_FIELDS \
	Gender gender; \
	simClock wasBorn; \
	simClock fertilityStart; \
	simClock fertilityEnd; \
	Children children;
// put properties of living entities here

struct LivingEntity {
	ENTITY_FIELDS
	LIVING_ENTITY_FIELDS
};

struct Human {
	ENTITY_FIELDS
	LIVING_ENTITY_FIELDS
	// put human-only properties here
};

struct Infected {
	ENTITY_FIELDS
	LIVING_ENTITY_FIELDS
	simClock becameInfected;
// put infected-only properties here
};

struct Zombie {
	ENTITY_FIELDS
	simClock becameZombie;
// put zombie-only properties here
};

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new random human who was born prior to start of the simulation.
 * If the human is a female, she can be pregnant.
 */
Human * newHuman(simClock clock);

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new zombie which was created at the time when simulation started.
 */
Zombie * newZombie(simClock clock);

/**
 * Converts a human into infected.
 * All attributes are preserved.
 */
Infected * toInfected(Human * human, simClock clock);

/**
 * Converts an infected into zombie.
 */
Zombie * toZombie(Infected * infected, simClock clock);

/**
 * Copies human into a new human, preserves all attributes.
 */
Human * copyHuman(Human * human);

/**
 * Copies infected into a new infected, preserves all attributes.
 */
Infected * copyInfected(Infected * infected);

/**
 * Copies zombie into a new zombie, preserves all attributes.
 */
Zombie * copyZombie(Zombie * zombie);

/**
 * Copies general entity into a new entity, preserves all attributes.
 */
Entity * copyEntity(Entity * entity);

/**
 * Copies general living entity into a new living entity, preserves all attributes.
 */
LivingEntity * copyLiving(LivingEntity * living);

/**
 * Conceives up to three children in mother's body.
 * The fertilization will happen with a probability.
 * Call this whenever a MALE is next to a FEMALE.
 */
void makeLove(LivingEntity * mother, LivingEntity * father, simClock clock, Stats stats);

/**
 * Mother gives birth to all her children when they are scheduled.
 * Type of children depends on mother's health condition.
 * Call this function repeatedly to born all children.
 */
LivingEntity * giveBirth(LivingEntity * mother, simClock clock);

/**
 * Returns the human back to allocator.
 */
void disposeHuman(Human * human);

/**
 * Returns the infected back to allocator.
 */
void disposeInfected(Infected * infected);

/**
 * Returns the zombie back to allocator.
 */
void disposeZombie(Zombie * zombie);

/**
 * Returns the entity back to allocator.
 */
void disposeEntity(Entity * entity);

/**
 * Frees all memory used by allocator.
 */
void destroyUnused();

/**
 * Returns maximal speed which the entity can go.
 */
double getMaxSpeed(Entity * entity, simClock clock);

/**
 * Returns probability of death for a living entity at given time.
 */
double getDeathRate(LivingEntity * living, simClock currentTime);

/**
 * Returns probability of decomposition of zombie at given time.
 */
double getDecompositionRate(Zombie * zombie, simClock currentTime);

#endif /* ENTITY_H_ */

// vim: ts=4 sw=4 et
