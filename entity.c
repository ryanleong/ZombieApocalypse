#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

#include "entity.h"
#include "common.h"
#include "random.h"
#include "constants.h"

typedef struct EntityAllocator {
	Entity * humans;
	Entity * infected;
	Entity * zombies;
} EntityAllocator;

EntityAllocator allocator = { NULL, NULL, NULL };

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
				entity = allocator.humans;
				allocator.humans = entity->asEntity;
			} else {
				entity = ((Entity *) malloc(sizeof(Human)));
			}
			break;
		case INFECTED:
			if (allocator.infected != NULL) {
				entity = allocator.infected;
				allocator.infected = entity->asEntity;
			} else {
				entity = ((Entity *) malloc(sizeof(Infected)));
			}
			break;
		case ZOMBIE:
			if (allocator.zombies != NULL) {
				entity = allocator.zombies;
				allocator.zombies = entity->asEntity;
			} else {
				entity = ((Entity *) malloc(sizeof(Zombie)));
			}
			break;
		}
	}
	entity->asEntity = entity;
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
Children newChildren(int count, simClock clock, bool already) {
	Children children;
	children.count = count;
	simClock event = randomEvent(PREGNANCY_DURATION_MEAN,
	PREGNANCY_DURATION_STD_DEV);
	if (already) {
		double rate = randomDouble();
		children.conceived = clock - rate * event;
		children.borns = MAX(1, clock + (1 - rate) * event); // in future!
	} else {
		children.conceived = clock;
		children.borns = clock + event;
	}

	return children;
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
			human->children = newChildren(count, clock, true);
		} else {
			human->children = NO_CHILDREN;
		}

		age = randomEvent(LIFE_EXPECTANCY_FEMALE_MEAN,
		LIFE_EXPECTANCY_FEMALE_STD_DEV);
		fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
		FERTILITY_START_FEMALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
		FERTILITY_END_FEMALE_STD_DEV);
	} else {
		human->gender = MALE;
		human->children = NO_CHILDREN;

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
	human->fertilityEnd = human->wasBorn + fertilityEnd;

	// the simulation start with a nice morning
	// when everybody woke up and nobody is tired
	human->lastSlept = clock;
	human->tiredness = 0;
	human->bearing = getRandomBearing();

	return human;
}

Zombie * newZombie(simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock; // right now
	simClock event = randomEvent(ZOMBIE_DECOMPOZITION_MEAN,
	ZOMBIE_DECOMPOZITION_STD_DEV);
	zombie->decomposes = clock + MAX(event, 1); // in future!
	zombie->bearing = getRandomBearing();

	return zombie;
}

Infected * toInfected(Human * human, simClock clock) {
	Infected * infected = newEntity(INFECTED)->asInfected;

	infected->gender = human->gender;
	infected->children = human->children;
	infected->lastSlept = human->lastSlept;
	infected->tiredness = human->tiredness;
	infected->wasBorn = human->wasBorn;
	infected->willDie = human->willDie;
	infected->bearing = human->bearing;
	infected->becameInfected = clock;

	simClock event = randomEvent(ZOMBIE_INFECTION_MEAN,
	ZOMBIE_INFECTION_STD_DEV);
	infected->becomesZombie = clock + MAX(1, event); // in future!

	return infected;
}

Zombie * toZombie(Infected * infected, simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock;

	simClock event = randomEvent(ZOMBIE_DECOMPOZITION_MEAN,
	ZOMBIE_DECOMPOZITION_STD_DEV);
	zombie->decomposes = clock + MAX(event, 1); // in future!

	zombie->bearing = getRandomBearing();

	return zombie;
}

Human * copyHuman(Human * human) {
	Human * h = newEntity(HUMAN)->asHuman;

	h->gender = human->gender;
	h->fertilityStart = human->fertilityStart;
	h->fertilityEnd = human->fertilityEnd;
	h->children = human->children;
	h->lastSlept = human->lastSlept;
	h->tiredness = human->tiredness;
	h->wasBorn = human->wasBorn;
	h->willDie = human->willDie;
	h->bearing = human->bearing;

	return h;
}

Infected * copyInfected(Infected * infected) {
	Infected * i = newEntity(INFECTED)->asInfected;

	i->gender = infected->gender;
	i->fertilityStart = infected->fertilityStart;
	i->fertilityEnd = infected->fertilityEnd;
	i->children = infected->children;
	i->lastSlept = infected->lastSlept;
	i->tiredness = infected->tiredness;
	i->wasBorn = infected->wasBorn;
	i->willDie = infected->willDie;
	i->becameInfected = infected->becameInfected;
	i->becomesZombie = infected->becomesZombie;
	i->bearing = infected->bearing;

	return i;
}

Zombie * copyZombie(Zombie * zombie) {
	Zombie * z = newEntity(ZOMBIE)->asZombie;

	z->becameZombie = zombie->becameZombie;
	z->decomposes = zombie->decomposes;
	z->bearing = zombie->bearing;

	return z;
}

Entity * copyEntity(Entity * entity) {
	switch (entity->type) {
	case HUMAN:
		return copyHuman(entity->asHuman)->asEntity;
	case INFECTED:
		return copyInfected(entity->asInfected)->asEntity;
	case ZOMBIE:
		return copyZombie(entity->asZombie)->asEntity;
	default: // won't happen
		return NULL;
	}
}

LivingEntity * copyLiving(LivingEntity * living) {
	return copyEntity(living->asEntity)->asLiving;
}

void makeLove(LivingEntity * mother, LivingEntity * father, simClock clock) {
	double rnd = randomDouble();
	if (rnd > PROBABILITY_FERTILIZATION) {
		return;
	}

	int count = randomCountOfUnborn();
	mother->children = newChildren(count, clock, false);
}

LivingEntity * giveBirth(LivingEntity * mother, simClock clock) {
	if (mother->children.count <= 0) {
		return NULL;
	}

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

	born->children = NO_CHILDREN;
	born->lastSlept = clock;

	born->tiredness = 1; // baby is tired
	born->wasBorn = clock;
	born->fertilityStart = clock + fertilityStart;
	born->fertilityEnd = clock + fertilityEnd;
	born->bearing = NO_BEARING;

	// loop through unborn children
	mother->children.count--;
	mother->tiredness = 1;
	return born;
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
			entity->asEntity = allocator.humans;
			allocator.humans = entity;
			break;
		case INFECTED:
			entity->asEntity = allocator.infected;
			allocator.infected = entity;
			break;
		case ZOMBIE:
			entity->asEntity = allocator.zombies;
			allocator.zombies = entity;
			break;
		}
	}
}

void disposeHuman(Human * human) {
	generalDispose(human->asEntity);
}

void disposeInfected(Infected * infected) {
	generalDispose(infected->asEntity);
}

void disposeZombie(Zombie * zombie) {
	generalDispose(zombie->asEntity);
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
	}
}

void destroyUnusedChain(Entity * entities) {
	// traverse the chain and free each element
	while (entities != NULL) {
		Entity * ptr = entities;
		entities = entities->asEntity;
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
	}
}

double getMaxSpeed(Entity * entity, simClock currentTime) {
	double moveChance = 0.0;

	if (entity->type == ZOMBIE) {
		Zombie * zombie = entity->asZombie;
		int zombieAge = (currentTime - zombie->becameZombie) / IN_YEARS;

		if (zombieAge < (ZOMBIE_DECOMPOZITION_MEAN / 2)) {
			moveChance = ZOMBIE_MOVE_SPEED_MEAN * 1.0;
		} else {
			moveChance = ZOMBIE_MOVE_SPEED_MEAN * 0.8;
		}
	} else {
		LivingEntity * living = entity->asLiving;
		int age = (currentTime - living->wasBorn) / IN_YEARS;

		moveChance = MALE_MOVE_SPEED_MEAN;

		if (age < 18) {
			moveChance = moveChance * MALE_UNDER_18_SPEED_MEAN;
		} else if (age > 39) {
			moveChance = moveChance * MALE_OVER_40_SPEED_MEAN;
		}

		if (living->gender == FEMALE) {
			moveChance = moveChance * FEMALE_TO_MALE_SPEED_RATIO;
		}
	}
	return moveChance;
}
