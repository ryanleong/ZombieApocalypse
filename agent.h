/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef AGENTS_H_
#define AGENTS_H_

#include "clock.h"

typedef enum EntityType {
	HUMAN, INFECTED, ZOMBIE
} EntityType;

typedef struct Entity Entity;

typedef struct LivingEntity LivingEntity;

typedef struct Human Human;

typedef struct Infected Infected;

typedef struct Zombie Zombie;

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
	union { \
		Entity * nextEntity; \
		LivingEntity * nextLiving; \
		Human * nextHuman; \
		Infected * nextInfected; \
		Zombie * nextZombie; \
	};

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
	double tiredness;

struct LivingEntity {
	ENTITY_FIELDS
	LIVING_ENTITY_FIELDS
};

struct Human {
	ENTITY_FIELDS
	LIVING_ENTITY_FIELDS
};

struct Infected {
	ENTITY_FIELDS
	LIVING_ENTITY_FIELDS
	simClock becameInfected;
	simClock becomesZombie;
};

struct Zombie {
	ENTITY_FIELDS
	simClock becameZombie;
	simClock decomposes;
};

#endif /* AGENTS_H_ */

// vim: ts=4 sw=4 et
