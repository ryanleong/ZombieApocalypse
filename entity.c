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
#ifdef _OPENMP
#pragma omp critical (EntityAllocatorRegion)
#endif
	{
		switch (type) {
		case HUMAN:
			if (allocator.humans != NULL) {
				entity = allocator.humans->asEntity;
				allocator.humans = entity->nextHuman;
			} else {
				entity = ((Entity *) malloc(sizeof(Human)));
			}
			break;
		case INFECTED:
			if (allocator.infected != NULL) {
				entity = allocator.infected->asEntity;
				allocator.infected = entity->nextInfected;
			} else {
				entity = ((Entity *) malloc(sizeof(Infected)));
			}
			break;
		case ZOMBIE:
			if (allocator.zombies != NULL) {
				entity = allocator.zombies->asEntity;
				allocator.zombies = entity->nextZombie;
			} else {
				entity = ((Entity *) malloc(sizeof(Zombie)));
			}
			break;
		case UNBORN:
			if (allocator.unborn != NULL) {
				entity = allocator.unborn->asEntity;
				allocator.unborn = entity->nextUnborn;
			} else {
				entity = ((Entity *) malloc(sizeof(Unborn)));
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

Human * newHuman(simClock clock) {
	Human * human = newEntity(HUMAN)->asHuman;

	double rnd = randomDouble();
	simClock age;
	simClock fertilityStart;
	simClock fertilityEnd;
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
		fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
		FERTILITY_START_FEMALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
		FERTILITY_END_FEMALE_STD_DEV);
	} else {
		human->gender = MALE;
		human->fetuses = NULL;

		age = randomEvent(LIFE_EXPECTANCY_MALE_MEAN,
		LIFE_EXPECTANCY_MALE_STD_DEV);
		fertilityStart = randomEvent(FERTILITY_START_MALE_MEAN,
		FERTILITY_START_MALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_MALE_MEAN,
		FERTILITY_END_MALE_STD_DEV);
	}

	double ageRate = randomDouble();
	human->wasBorn = clock - ageRate * age;
	human->willDie = MAX(1, clock + (1 - ageRate) * age); // in future!

	// may or may not be in future
	human->fertilityStart = human->wasBorn + fertilityStart;
	human->fertilityStart = human->wasBorn + fertilityEnd;

	// the simulation start with a nice morning
	// when everybody woke up and nobody is tired
	human->lastSlept = clock;
	human->tiredness = 0;

	return human;
}

Zombie * newZombie(simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock; // right now
	simClock event = randomEvent(ZOMBIE_DECOMPOZITION_MEAN,
	ZOMBIE_DECOMPOZITION_STD_DEV);
	zombie->decomposes = clock + MAX(event, 1); // in future!

	return zombie;
}

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

Zombie * toZombie(Infected * infected, simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock;

	simClock event = randomEvent(ZOMBIE_DECOMPOZITION_MEAN,
	ZOMBIE_DECOMPOZITION_STD_DEV);
	zombie->decomposes = clock + MAX(event, 1); // in future!

	disposeInfected(infected);
	return zombie;
}

Human * copyHuman(Human * human) {
	Human * h = newEntity(HUMAN)->asHuman;

	h->gender = human->gender;
	h->fertilityStart = human->fertilityStart;
	h->fertilityEnd = human->fertilityEnd;

	h->fetuses = NULL;
	for (Unborn * u = human->fetuses; u != NULL; u = u->nextUnborn) {
		Unborn * u_ = copyUnborn(u);
		u_->nextUnborn = h->fetuses;
		h->fetuses = u_;
	}

	h->lastSlept = human->lastSlept;
	h->tiredness = human->tiredness;
	h->wasBorn = human->wasBorn;
	h->willDie = human->willDie;

	return h;
}

Infected * copyInfected(Infected * infected) {
	Infected * i = newEntity(INFECTED)->asInfected;

	i->gender = infected->gender;
	i->fertilityStart = infected->fertilityStart;
	i->fertilityEnd = infected->fertilityEnd;

	i->fetuses = NULL;
	for (Unborn * u = infected->fetuses; u != NULL; u = u->nextUnborn) {
		Unborn * u_ = copyUnborn(u);
		u_->nextUnborn = i->fetuses;
		i->fetuses = u_;
	}

	i->lastSlept = infected->lastSlept;
	i->tiredness = infected->tiredness;
	i->wasBorn = infected->wasBorn;
	i->willDie = infected->willDie;
	i->becameInfected = infected->becameInfected;
	i->becomesZombie = infected->becomesZombie;

	return i;
}

Zombie * copyZombie(Zombie * zombie) {
	Zombie * z = newEntity(ZOMBIE)->asZombie;

	z->becameZombie = zombie->becameZombie;
	z->decomposes = zombie->decomposes;

	return z;
}

Unborn * copyUnborn(Unborn * unborn) {
	Unborn * u = newEntity(UNBORN)->asUnborn;

	u->borns = unborn->borns;
	u->conceived = unborn->conceived;

	return u;
}

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

LivingEntity * copyLiving(LivingEntity * living) {
	return copyEntity(living->asEntity)->asLiving;
}

void makeLove(LivingEntity * mother, LivingEntity * father, simClock clock) {
	// correct gender and not pregnant
	if (mother->gender == FEMALE && mother->fetuses == NULL
			&& father->gender == MALE) {
		// both are fertile
		if (clock >= mother->fertilityStart && clock < mother->fertilityEnd
				&& clock >= father->fertilityStart
				&& clock < father->fertilityEnd) {
			int count = randomCountOfUnborn();
			mother->fetuses = newUnborn(count, clock, false);
		}
	}
}

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
		simClock fertilityStart;
		simClock fertilityEnd;
		if (rnd < MALE_FEMALE_RATE) {
			born->gender = FEMALE;
			age = randomEvent(LIFE_EXPECTANCY_FEMALE_MEAN,
			LIFE_EXPECTANCY_FEMALE_STD_DEV);
			fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
			FERTILITY_START_FEMALE_STD_DEV);
			fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
			FERTILITY_END_FEMALE_STD_DEV);
		} else {
			born->gender = MALE;
			age = randomEvent(LIFE_EXPECTANCY_MALE_MEAN,
			LIFE_EXPECTANCY_MALE_STD_DEV);
			fertilityStart = randomEvent(FERTILITY_START_MALE_MEAN,
			FERTILITY_START_MALE_STD_DEV);
			fertilityEnd = randomEvent(FERTILITY_END_MALE_MEAN,
			FERTILITY_END_MALE_STD_DEV);
		}

		born->willDie = MAX(1, clock + age); // in future!

		born->fetuses = NULL;
		born->lastSlept = clock;

		born->tiredness = 1; // baby is tired
		born->wasBorn = clock;
		born->fertilityStart = clock + fertilityStart;
		born->fertilityStart = clock + fertilityEnd;

		// loop through unborn children
		Unborn * bornFetus = mother->fetuses;
		mother->fetuses = bornFetus->nextUnborn;
		disposeUnborn(bornFetus);

		born->nextLiving = chain;
		chain = born;
	}

	mother->tiredness = 1; // mother is tired

	return chain;
}

/**
 * The entity must be disposed first!
 * Returns the entity back to the allocator.
 */
void generalDispose(Entity * entity) {
#ifdef _OPENMP
#pragma omp critical (EntityAllocatorRegion)
#endif
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

void disposeHuman(Human * human) {
	if (human->fetuses != NULL) {
		disposeEntities(human->fetuses->asEntity);
	}
	generalDispose(human->asEntity);
}

void disposeInfected(Infected * infected) {
	if (infected->fetuses != NULL) {
		disposeEntities(infected->fetuses->asEntity);
	}
	generalDispose(infected->asEntity);
}

void disposeZombie(Zombie * zombie) {
	generalDispose(zombie->asEntity);
}

void disposeUnborn(Unborn * unborn) {
	generalDispose(unborn->asEntity);
}

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

void disposeEntities(Entity * entities) {
	// traverse the chain and dispose each element
	while (entities != NULL) {
		Entity * ptr = entities;
		entities = entities->nextEntity;
		disposeEntity(ptr);
	}
}

void destroyUnusedChain(Entity * entities) {
	// traverse the chain and free each element
	while (entities != NULL) {
		Entity * ptr = entities;
		entities = entities->nextEntity;
		free(ptr);
	}
}

void destroyUnused() {
	// we need to lock the access to allocator
#ifdef _OPENMP
#pragma omp critical (EntityAllocatorRegion)
#endif
	{
		destroyUnusedChain(allocator.humans->asEntity);
		destroyUnusedChain(allocator.infected->asEntity);
		destroyUnusedChain(allocator.zombies->asEntity);
		destroyUnusedChain(allocator.unborn->asEntity);
	}
}
