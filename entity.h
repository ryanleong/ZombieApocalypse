/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include "clock.h"

typedef enum EntityType {
	UNBORN, HUMAN, INFECTED, ZOMBIE
} EntityType;

typedef struct Entity Entity;

typedef struct LivingEntity LivingEntity;

typedef struct Human Human;

typedef struct Infected Infected;

typedef struct Zombie Zombie;

typedef struct Unborn Unborn;

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
		Unborn * asUnborn; \
	}; \
	union { \
		Entity * nextEntity; \
		LivingEntity * nextLiving; \
		Human * nextHuman; \
		Infected * nextInfected; \
		Zombie * nextZombie; \
		Unborn * nextUnborn; \
	}; \
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
	Unborn * fetuses; \
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

struct Unborn {
	ENTITY_FIELDS
	simClock conceived;
	simClock borns;
// put unborn-only properties here
};

Human * newHuman(simClock clock);

Zombie * newZombie(simClock clock);

Infected * toInfected(Human * human, simClock clock);

Zombie * toZombie(Infected * infected, simClock clock);

Human * copyHuman(Human * human);

Infected * copyInfected(Infected * infected);

Zombie * copyZombie(Zombie * zombie);

Unborn * copyUnborn(Unborn * unborn);

Entity * copyEntity(Entity * entity);

void makeLove(LivingEntity * mother, LivingEntity * father, simClock clock);

LivingEntity * giveBirth(LivingEntity * mother, simClock clock);

void disposeHuman(Human * human);

void disposeInfected(Infected * infected);

void disposeZombie(Zombie * zombie);

void disposeUnborn(Unborn * unborn);

void disposeEntity(Entity * entity);

void disposeEntities(Entity * entities);

void destroyUnused();

#endif /* ENTITY_H_ */

// vim: ts=4 sw=4 et
