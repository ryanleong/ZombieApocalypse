/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include "clock.h"

typedef enum EntityType {
	HUMAN, INFECTED, ZOMBIE
} EntityType;

typedef struct Entity Entity;

typedef struct LivingEntity LivingEntity;

typedef struct Human Human;

typedef struct Infected Infected;

typedef struct Zombie Zombie;

typedef struct Children {
	int count;
	simClock conceived;
	simClock borns;
} Children;

#define NO_CHILDREN ((const struct Children){0, 0, 0})

/*
 * First field is an enum specifying type of entity.
 * Second field is a pointer to itself to simplify "subclass" access.
 * Third field is a pointer to next (generally it may be of some other type).
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
	Entity * next; \
// put general entity properties here

typedef enum Gender {
	MALE, FEMALE
} Gender;

typedef struct Entity {
	ENTITY_FIELDS
} Entity;

#define LIVING_ENTITY_FIELDS \
	Gender gender; \
	simClock wasBorn; \
	simClock willDie; \
	simClock lastSlept; \
	double tiredness; \
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
	simClock becomesZombie;
// put infected-only properties here
};

struct Zombie {
	ENTITY_FIELDS
	simClock becameZombie;
	simClock decomposes;
// put zombie-only properties here
};

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new random human who lives prior to start of the simulation.
 * If the human is a female, she can be pregnant.
 */
Human * newHuman(simClock clock);

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new zombie which will eventually decompose.
 */
Zombie * newZombie(simClock clock);

/**
 * Converts a human into infected.
 * All attributes are preserved and becoming zombie is planned.
 * The human is disposed.
 */
Infected * toInfected(Human * human, simClock clock);

/**
 * Converts an infected into zombie.
 * The infected is disposed.
 */
Zombie * toZombie(Infected * infected, simClock clock);

/**
 * Copies human into a new entity, preserves all attributes.
 */
Human * copyHuman(Human * human);

/**
 * Copies infected into a new entity, preserves all attributes.
 */
Infected * copyInfected(Infected * infected);

/**
 * Copies zombie into a new entity, preserves all attributes.
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
 */
void makeLove(LivingEntity * mother, LivingEntity * father, simClock clock);

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
 * Frees all used memory by allocator.
 */
void destroyUnused();

#endif /* ENTITY_H_ */

// vim: ts=4 sw=4 et
