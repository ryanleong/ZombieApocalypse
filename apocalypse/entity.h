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
 * An Entity is an agent in the World.
 * All attributes are inside this one structure.
 * To preserve space, we use bit-fields, which makes this structure  24 Bytes long;
 * it also constraints us to simulation of 2^31 steps and maximal age 2^21 steps.
 * Which is for 1 step a day: 5883516 years of simulation and 5745 years of age.
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

typedef Entity * EntityPtr;

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new random human who was born prior to start of the simulation.
 * If the human is a female, she can be pregnant.
 */
void newHuman(EntityPtr human, simClock clock);

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new zombie which was created at the time when simulation started.
 */
void newZombie(EntityPtr zombie, simClock clock);

/**
 * Converts a human into infected.
 * All attributes are preserved.
 */
void toInfected(EntityPtr infected, simClock clock);

/**
 * Converts an infected into zombie.
 */
void toZombie(EntityPtr zombie, simClock clock);

/**
 * Conceives up to three children in mother's body.
 * The fertilization will happen with a probability.
 * Call this whenever a MALE is next to a FEMALE.
 */
void makeLove(EntityPtr mother, EntityPtr father, simClock clock, Stats stats);

/**
 * Mother gives birth to all her children when they are scheduled.
 * Type of children depends on mother's health condition.
 * Call this function repeatedly to born all children.
 */
Entity giveBirth(EntityPtr mother, simClock clock);

/**
 * Returns maximal speed which the entity can go.
 */
double getMaxSpeed(EntityPtr entity, simClock clock);

/**
 * Returns probability of death for a living entity at given time.
 */
double getDeathRate(EntityPtr living, simClock currentTime);

/**
 * Returns probability of decomposition of zombie at given time.
 */
double getDecompositionRate(EntityPtr zombie, simClock currentTime);

#endif /* ENTITY_H_ */

// vim: ts=4 sw=4 et
