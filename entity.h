/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include "clock.h"
#include "direction.h"
#include "stats.h"

/**
 * Entity may be of type HUMAN, INFECTED, ZOMBIE or NONE.
 * Each entity has its own attributes but some of them are shared.
 * HUMAN and INFECTED are together a LivingEntity.
 */
typedef enum EntityType {
	NONE, HUMAN, INFECTED, ZOMBIE
} EntityType;

/**
 * We have only two genders - MALE and FEMALE.
 * Sorry http://en.wikipedia.org/wiki/Third_gender
 */
typedef enum Gender {
	MALE, FEMALE
} Gender;

/**
 * An Entity is an agent in the World
 */
typedef struct Entity {
	// this group has 32 bits in total; last 6 bits is padding
	EntityType type :2; // 0, 1, 2, 3
	Gender gender :1; // 0, 1
	unsigned int children :2; // 0, 1, 2, 3
	simClockDelta borns :21;
	unsigned :6; // 6 bits are free

	simClock origin :32;
	unsigned :0;

	// these 3 delta clocks are 63 bits + 1 bit padding
	simClockDelta fertilityStart :21;
	simClockDelta fertilityEnd :21;
	simClockDelta becameInfected :21;
	unsigned :0;

	bearing bearing; // size is 2*sizeof(float) = 8
} Entity;

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new random human who was born prior to start of the simulation.
 * If the human is a female, she can be pregnant.
 */
void newHuman(Entity * entity, simClock clock);

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new zombie which was created at the time when simulation started.
 */
void newZombie(Entity * entity, simClock clock);

/**
 * Converts a human into infected.
 * All attributes are preserved.
 */
void toInfected(Entity * human, simClock clock);

/**
 * Converts an infected into zombie.
 */
void toZombie(Entity * infected, simClock clock);

/**
 * Conceives up to three children in mother's body.
 * The fertilization will happen with a probability.
 * Call this whenever a MALE is next to a FEMALE.
 */
void makeLove(Entity * mother, Entity * father, simClock clock, Stats stats);

/**
 * Mother gives birth to all her children when they are scheduled.
 * Type of children depends on mother's health condition.
 * Call this function repeatedly to born all children.
 */
Entity giveBirth(Entity * mother, simClock clock);

/**
 * Returns maximal speed which the entity can go.
 */
double getMaxSpeed(Entity * entity, simClock clock);

/**
 * Returns probability of death for a living entity at given time.
 */
double getDeathRate(Entity * living, simClock currentTime);

/**
 * Returns probability of decomposition of zombie at given time.
 */
double getDecompositionRate(Entity * zombie, simClock currentTime);

#endif /* ENTITY_H_ */

// vim: ts=4 sw=4 et
