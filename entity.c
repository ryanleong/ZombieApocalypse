#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

#include "entity.h"
#include "common.h"
#include "random.h"
#include "constants.h"

typedef struct EntityAllocator {
	Human * humans;
	Infected * infected;
	Zombie * zombies;
	Unborn * unborn;
} EntityAllocator;

EntityAllocator allocator = { NULL, NULL, NULL, NULL };

/**
 * Allocates new entity in memory if none can be recycled.
 * It also fills pointer to next = NULL and as = this and also fills type.
 * Other fields are uninitialized!
 */
Entity * newEntity(EntityType type) {
	Entity * entity = NULL;
#pragma omp critical (EntityAllocatorRegion)
	{
		switch (type) {
		case HUMAN:
			if (allocator.humans != NULL) {
				entity = allocator.humans->asEntity;
				allocator.humans = entity->nextHuman;
			} else {
				entity = ((Human *) malloc(sizeof(Human)))->asEntity;
			}
			break;
		case INFECTED:
			if (allocator.infected != NULL) {
				entity = allocator.infected->asEntity;
				allocator.infected = entity->nextInfected;
			} else {
				entity = ((Infected*) malloc(sizeof(Infected)))->asEntity;
			}
			break;
		case ZOMBIE:
			if (allocator.zombies != NULL) {
				entity = allocator.zombies->asEntity;
				allocator.zombies = entity->nextZombie;
			} else {
				entity = ((Zombie*) malloc(sizeof(Zombie)))->asEntity;
			}
			break;
		case UNBORN:
			if (allocator.unborn != NULL) {
				entity = allocator.unborn->asEntity;
				allocator.unborn = entity->nextUnborn;
			} else {
				entity = ((Unborn*) malloc(sizeof(Unborn)))->asEntity;
			}
			break;
		}
	}
	entity->asEntity = entity;
	entity->nextEntity = NULL;
	entity->type = type;
	return entity;
}

/**
 * Returns random number of children.
 */
int randomCountOfUnborn() {
	double rnd = drand48();
	if (rnd < PROBABILITY_ONE_CHILD) {
		return 1;
	} else if (rnd < PROBABILITY_TWO_CHILDREN) {
		return 2;
	} else {
		return 3;
	}
}

/**
 * Creates chain of unborn.
 * Already means that it is the start of the simulation.
 */
Unborn * newUnborn(int count, simClock clock, bool already) {
	Unborn * chain = NULL;
	for (int i = 0; i < count; i++) {
		Unborn * unborn = newEntity(UNBORN)->asUnborn;
		simClock event = randomEvent(PREGNANCY_DURATION_MEAN,
		PREGNANCY_DURATION_STD_DEV);
		if (already) {
			double rate = randomDouble();
			unborn->conceived = clock - rate * event;
			unborn->borns = MAX(1, clock + (1 - rate) * event); // in future!
		} else {
			unborn->conceived = clock;
			unborn->borns = clock + event;
		}

		unborn->nextUnborn = chain;
		chain = unborn;
	}
	return chain;
}

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new random human who lives prior to start of the simulation.
 * If the human is a female, she can be pregnant.
 */
Human * newHuman(simClock clock) {
	Human * human = newEntity(HUMAN)->asHuman;

	double rnd = randomDouble();
	simClock age;
	if (rnd < MALE_FEMALE_RATE) {
		human->gender = FEMALE;

		if (randomDouble() < PROBABILITY_INITIAL_PREGNANCY) {
			int count = randomCountOfUnborn();
			human->fetuses = newUnborn(count, clock, true);
		} else {
			human->fetuses = NULL;
		}

		age = randomEvent(LIFE_EXPECTANCY_FEMALE_MEAN,
		LIFE_EXPECTANCY_FEMALE_STD_DEV);
	} else {
		human->gender = MALE;
		human->fetuses = NULL;

		age = randomEvent(LIFE_EXPECTANCY_MALE_MEAN,
		LIFE_EXPECTANCY_MALE_STD_DEV);
	}

	double ageRate = randomDouble();
	human->wasBorn = clock - ageRate * age;
	human->willDie = MAX(1, clock + (1 - ageRate) * age); // in future!

	// TODO lastSlept, tiredness

	return human;
}

/**
 * Use only at the start of the simulation (clock should be 0)
 * Creates a new zombie which will eventually decompose.
 */
Zombie * newZombie(simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock; // right now
	simClock event = randomEvent(ZOMBIE_DECOMPOZITION_MEAN,
	ZOMBIE_DECOMPOZITION_STD_DEV);
	zombie->decomposes = clock + MAX(event, 1); // in future!

	return zombie;
}

/**
 * Converts a human into infected.
 * All attributes are preserved and becoming zombie is planned.
 * The human is disposed.
 */
Infected * toInfected(Human * human, simClock clock) {
	Infected * infected = newEntity(INFECTED)->asInfected;

	infected->gender = human->gender;
	infected->fetuses = human->fetuses;
	human->fetuses = NULL; // fetuses are reparented to this infected

	infected->lastSlept = human->lastSlept;
	infected->tiredness = human->tiredness;
	infected->wasBorn = human->wasBorn;
	infected->willDie = human->willDie;

	infected->becameInfected = clock;

	simClock event = randomEvent(ZOMBIE_INFECTION_MEAN,
	ZOMBIE_INFECTION_STD_DEV);
	infected->becomesZombie = MAX(1, event); // in future!

	disposeHuman(human);
	return infected;
}

/**
 * Converts an infected into zombie.
 * The infected is disposed.
 */
Zombie * toZombie(Infected * infected, simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock;

	simClock event = randomEvent(ZOMBIE_DECOMPOZITION_MEAN,
	ZOMBIE_DECOMPOZITION_STD_DEV);
	zombie->decomposes = clock + MAX(event, 1); // in future!

	disposeInfected(infected);
	return zombie;
}

/**
 * Copies human into a new entity, preserves all attributes.
 */
Human * copyHuman(Human * human) {
	Human * h = newEntity(HUMAN)->asHuman;

	h->gender = human->gender;
	h->fetuses = human->fetuses;
	h->lastSlept = human->lastSlept;
	h->tiredness = human->tiredness;
	h->wasBorn = human->wasBorn;
	h->willDie = human->willDie;

	return h;
}

/**
 * Copies infected into a new entity, preserves all attributes.
 */
Infected * copyInfected(Infected * infected) {
	Infected * i = newEntity(INFECTED)->asInfected;

	i->gender = infected->gender;
	i->fetuses = infected->fetuses;
	i->lastSlept = infected->lastSlept;
	i->tiredness = infected->tiredness;
	i->wasBorn = infected->wasBorn;
	i->willDie = infected->willDie;
	i->becameInfected = infected->becameInfected;
	i->becomesZombie = infected->becomesZombie;

	return i;
}

/**
 * Copies zombie into a new entity, preserves all attributes.
 */
Zombie * copyZombie(Zombie * zombie) {
	Zombie * z = newEntity(ZOMBIE)->asZombie;

	z->becameZombie = zombie->becameZombie;
	z->decomposes = zombie->decomposes;

	return z;
}

/**
 * Copies unborn into a new entity, preserves all attributes.
 */
Unborn * copyUnborn(Unborn * unborn) {
	Unborn * u = newEntity(UNBORN)->asUnborn;

	u->borns = unborn->borns;
	u->conceived = unborn->conceived;

	return u;
}

/**
 * Copies general entity into a new entity, preserves all attributes.
 */
Entity * copyEntity(Entity * entity) {
	switch (entity->type) {
	case HUMAN:
		return copyHuman(entity->asHuman)->asEntity;
	case INFECTED:
		return copyInfected(entity->asInfected)->asEntity;
	case ZOMBIE:
		return copyZombie(entity->asZombie)->asEntity;
	case UNBORN:
		return copyUnborn(entity->asUnborn)->asEntity;
	default: // won't happen
		return NULL;
	}
}

/**
 * Conceives up to three children in mother's body.
 */
void makeLove(LivingEntity * mother, LivingEntity * father, simClock clock) {
	if (mother->gender == FEMALE && mother->fetuses == NULL
			&& father->gender == MALE) {
		int count = randomCountOfUnborn();
		mother->fetuses = newUnborn(count, clock, false);
	}
}

/**
 * Mother gives birth to all her children when they are scheduled.
 * Type of children depends on mother's health condition.
 */
LivingEntity * giveBirth(LivingEntity * mother, simClock clock) {
	LivingEntity * chain = NULL;
	while (mother->fetuses != NULL) {
		LivingEntity * born = NULL;
		switch (mother->type) {
		case HUMAN: {
			Human * human = newEntity(HUMAN)->asHuman;
			born = human->asLiving;
			break;
		}
		case INFECTED: {
			Infected * infected = newEntity(INFECTED)->asInfected;
			infected->becameInfected = mother->asInfected->becameInfected;
			infected->becomesZombie = mother->asInfected->becomesZombie;
			born = infected->asLiving;
			break;
		}
		default: // won't happen, silent GCC
			break;
		}

		double rnd = randomDouble();
		simClock age;
		if (rnd < MALE_FEMALE_RATE) {
			born->gender = FEMALE;
			age = randomEvent(LIFE_EXPECTANCY_FEMALE_MEAN,
			LIFE_EXPECTANCY_FEMALE_STD_DEV);
		} else {
			born->gender = MALE;
			age = randomEvent(LIFE_EXPECTANCY_MALE_MEAN,
			LIFE_EXPECTANCY_MALE_STD_DEV);
		}

		born->willDie = MAX(1, clock + age); // in future!

		born->fetuses = NULL;
		born->lastSlept = clock;

		// TODO tiredness

		born->wasBorn = clock;

		// loop through unborn children
		Unborn * bornFetus = mother->fetuses;
		mother->fetuses = bornFetus->nextUnborn;
		disposeUnborn(bornFetus);

		born->nextLiving = chain;
		chain = born;
	}

	return chain;
}

/**
 * The entity must be disposed first!
 * Returns the entity back to the allocator.
 */
void generalDispose(Entity * entity) {
#pragma omp critical (EntityAllocatorRegion)
	{

		switch (entity->type) {
		case HUMAN:
			entity->nextHuman = allocator.humans;
			allocator.humans = entity->asHuman;
			break;
		case INFECTED:
			entity->nextInfected = allocator.infected;
			allocator.infected = entity->asInfected;
			break;
		case ZOMBIE:
			entity->nextZombie = allocator.zombies;
			allocator.zombies = entity->asZombie;
			break;
		case UNBORN:
			entity->nextUnborn = allocator.unborn;
			allocator.unborn = entity->asUnborn;
			break;
		}
	}
}

/**
 * Returns the human back to allocator.
 */
void disposeHuman(Human * human) {
	if (human->fetuses) {
		disposeEntities(human->fetuses->asEntity);
	}
	generalDispose(human->asEntity);
}

/**
 * Returns the infected back to allocator.
 */
void disposeInfected(Infected * infected) {
	if (infected->fetuses) {
		disposeEntities(infected->fetuses->asEntity);
	}
	generalDispose(infected->asEntity);
}

/**
 * Returns the zombie back to allocator.
 */
void disposeZombie(Zombie * zombie) {
	generalDispose(zombie->asEntity);
}

/**
 * Returns the unborn back to allocator.
 */
void disposeUnborn(Unborn * unborn) {
	generalDispose(unborn->asEntity);
}

/**
 * Returns the entity back to allocator.
 */
void disposeEntity(Entity * entity) {
	switch (entity->type) {
	case HUMAN:
		disposeHuman(entity->asHuman);
		break;
	case INFECTED:
		disposeInfected(entity->asInfected);
		break;
	case ZOMBIE:
		disposeZombie(entity->asZombie);
		break;
	case UNBORN:
		disposeUnborn(entity->asUnborn);
		break;
	}
}

/**
 * Returns the whole chain of entities back to allocator.
 * Argument may be NULL.
 */
void disposeEntities(Entity * entities) {
	while (entities != NULL) {
		Entity * ptr = entities;
		entities = entities->nextEntity;
		disposeEntity(ptr);
	}
}

void destroyUnusedChain(Entity * entities) {
	while (entities != NULL) {
		Entity * ptr = entities;
		entities = entities->nextEntity;
		free(ptr);
	}
}

/**
 * Frees all the
 */
void destroyUnused() {
#pragma omp critical (EntityAllocatorRegion)
	{
		destroyUnusedChain(allocator.humans->asEntity);
		destroyUnusedChain(allocator.infected->asEntity);
		destroyUnusedChain(allocator.zombies->asEntity);
		destroyUnusedChain(allocator.unborn->asEntity);
	}
}
