#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

#include "entity.h"
#include "common.h"
#include "random.h"
#include "constants.h"

/**
 * Contains linked lists of entities of different types.
 * EntityAllocator wraps malloc; its purpose is to reuse old entities.
 * All entities are created and removed in each step of the simulation.
 *
 * In allocator, despite their names the asXYZ pointers are used to point to next entity.
 */
typedef struct EntityAllocator {
	Human * humans;
	Infected * infected;
	Zombie * zombies;
} EntityAllocator;

/**
 * There is only one allocator per thread.
 */
EntityAllocator allocator; // initialized to { NULL, NULL, NULL };
#ifdef _OPENMP
#pragma omp threadprivate(allocator)
#endif

/**
 * Allocates new entity in memory if none can be recycled.
 * It fills pointer asXYZ = this and also fills type.
 * Other fields are uninitialized!
 *
 */
static Entity * newEntity(EntityType type) {
	Entity * entity = NULL;
	switch (type) {
	case HUMAN:
		if (allocator.humans != NULL) {
			entity = (Entity *) allocator.humans;
			allocator.humans = entity->asHuman;
		} else {
			entity = ((Entity *) malloc(sizeof(Human)));
		}
		break;

	case INFECTED:
		if (allocator.infected != NULL) {
			entity = (Entity *) allocator.infected;
			allocator.infected = entity->asInfected;
		} else {
			entity = ((Entity *) malloc(sizeof(Infected)));
		}
		break;

	case ZOMBIE:
		if (allocator.zombies != NULL) {
			entity = (Entity *) allocator.zombies;
			allocator.zombies = entity->asZombie;
		} else {
			entity = ((Entity *) malloc(sizeof(Zombie)));
		}
		break;
	}
	entity->asEntity = entity;
	entity->type = type;
	return entity;
}

/**
 * Returns random number of children.
 */
static int randomCountOfUnborn() {
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
static Children newChildren(int count, simClock clock, bool already) {
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
	simClock fertilityStart;
	simClock fertilityEnd;
	if (rnd < FEMALE_MALE_RATIO) {
		human->gender = FEMALE;

		if (randomDouble() < PROBABILITY_INITIAL_PREGNANCY) {
			int count = randomCountOfUnborn();
			human->children = newChildren(count, clock, true);
		} else {
			human->children = NO_CHILDREN;
		}

		fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
		FERTILITY_START_FEMALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
		FERTILITY_END_FEMALE_STD_DEV);
	} else {
		human->gender = MALE;
		human->children = NO_CHILDREN;
		fertilityStart = randomEvent(FERTILITY_START_MALE_MEAN,
		FERTILITY_START_MALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_MALE_MEAN,
		FERTILITY_END_MALE_STD_DEV);
	}

	// TODO make the new human age dependent on age distribution, low priority
	//if (human->gender == FEMALE) {
        double try=randomDouble();
    
        if(try<CHILD_POP){
            human->wasBorn = clock - (randomDouble() * HUMAN_CHILD_YOUNG_BORDER);
        }else if(try<YOUNG_POP+CHILD_POP){
            human->wasBorn = clock - HUMAN_CHILD_YOUNG_BORDER - randomDouble() * YOUNG_PERIOD;
        }else if(try<MIDDLEAGE_POP+YOUNG_POP+CHILD_POP){
            human->wasBorn = clock - HUMAN_YOUNG_MIDDLEAGE_BORDER - randomDouble() * MIDDLEAGE_PERIOD;//65-37.5=27.5
        }else{
            human->wasBorn = clock - HUMAN_MIDDLEAGE_ELDERLY_BORDER - randomDouble() * ELDER_PERIOD;//110-65=45
        }
	//} else {
		//human->wasBorn = clock - randomDouble() * LIFE_EXPECTANCY_MALE_MEAN;
	//}

// may or may not be in future
	human->fertilityStart = human->wasBorn + fertilityStart;
	human->fertilityEnd = human->wasBorn + fertilityEnd;

	human->bearing = getRandomBearing();

	return human;
}

Zombie * newZombie(simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock; // right now
	zombie->bearing = getRandomBearing();

	return zombie;
}

Infected * toInfected(Human * human, simClock clock) {
	Infected * infected = newEntity(INFECTED)->asInfected;

	infected->gender = human->gender;
	infected->children = human->children;
	infected->wasBorn = human->wasBorn;
	infected->bearing = human->bearing;
	infected->becameInfected = clock;

	return infected;
}

Zombie * toZombie(Infected * infected, simClock clock) {
	Zombie * zombie = newEntity(ZOMBIE)->asZombie;

	zombie->becameZombie = clock;
	// after becoming zombie, the bearing is restarted
	zombie->bearing = getRandomBearing();

	return zombie;
}

Human * copyHuman(Human * human) {
	Human * h = newEntity(HUMAN)->asHuman;

	h->gender = human->gender;
	h->fertilityStart = human->fertilityStart;
	h->fertilityEnd = human->fertilityEnd;
	h->children = human->children;
	h->wasBorn = human->wasBorn;
	h->bearing = human->bearing;

	return h;
}

Infected * copyInfected(Infected * infected) {
	Infected * i = newEntity(INFECTED)->asInfected;

	i->gender = infected->gender;
	i->fertilityStart = infected->fertilityStart;
	i->fertilityEnd = infected->fertilityEnd;
	i->children = infected->children;
	i->wasBorn = infected->wasBorn;
	i->becameInfected = infected->becameInfected;
	i->bearing = infected->bearing;

	return i;
}

Zombie * copyZombie(Zombie * zombie) {
	Zombie * z = newEntity(ZOMBIE)->asZombie;

	z->becameZombie = zombie->becameZombie;
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

void makeLove(LivingEntity * mother, LivingEntity * father, simClock clock, Stats stats) {
    int children = stats.humanFemalesDied + stats.humanMalesDied;
    int couples = stats.couplesMakingLove;
    if(couples == 0) {
        return;
    }
    
	double rnd = randomDouble();
    
    /*if(rnd > children / (double)couples) {
        return;
    }*/
    
    if(stats.humanFemales + stats.humanMales>12000){
        if(rnd > children / (double)couples) {
            return;
        }
    }else{
        if (rnd > 10*PROBABILITY_FERTILIZATION) {//0.006 human stable
            return;
        }
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
		born = infected->asLiving;
		break;
	}
	default: // won't happen, silent GCC
		break;
	}

	double rnd = randomDouble();
	simClock fertilityStart;
	simClock fertilityEnd;
	if (rnd < FEMALE_MALE_RATIO) {
		born->gender = FEMALE;
		fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
		FERTILITY_START_FEMALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
		FERTILITY_END_FEMALE_STD_DEV);
	} else {
		born->gender = MALE;
		fertilityStart = randomEvent(FERTILITY_START_MALE_MEAN,
		FERTILITY_START_MALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_MALE_MEAN,
		FERTILITY_END_MALE_STD_DEV);
	}

	born->children = NO_CHILDREN;
	born->wasBorn = clock;
	born->fertilityStart = clock + fertilityStart;
	born->fertilityEnd = clock + fertilityEnd;
	born->bearing = NO_BEARING;

	// decrease number of unborn children
	mother->children.count--;
	return born;
}

void disposeHuman(Human * human) {
	human->asHuman = allocator.humans;
	allocator.humans = human;
}

void disposeInfected(Infected * infected) {
	infected->asInfected = allocator.infected;
	allocator.infected = infected;
}

void disposeZombie(Zombie * zombie) {
	zombie->asZombie = allocator.zombies;
	allocator.zombies = zombie;
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

/**
 * Traverses the chain and frees each element.
 */
void destroyUnusedChain(Entity * entities) {
	while (entities != NULL) {
		Entity * ptr = entities;
		entities = entities->asEntity;
		free(ptr);
	}
}

void destroyUnused() {
	if (allocator.humans != NULL) {
		destroyUnusedChain(allocator.humans->asEntity);
		allocator.humans = NULL;
	}
	if (allocator.infected != NULL) {
		destroyUnusedChain(allocator.infected->asEntity);
		allocator.infected = NULL;
	}
	if (allocator.zombies != NULL) {
		destroyUnusedChain(allocator.zombies->asEntity);
		allocator.zombies = NULL;
	}
}

double getMaxSpeed(Entity * entity, simClock currentTime) {
	if (entity->type == ZOMBIE) {
		Zombie * zombie = entity->asZombie;
		int age = currentTime - zombie->becameZombie;
		if (age < ZOMBIE_YOUNG_OLD_BORDER) {
			return SPEED_ZOMBIE_YOUNG;
		} else {
			return SPEED_ZOMBIE_OLD;
		}
	} else {
		LivingEntity * living = entity->asLiving;
		int age = currentTime - living->wasBorn;
		if (living->gender == FEMALE) {
			if (age < HUMAN_CHILD_YOUNG_BORDER) {
				return SPEED_FEMALE_CHILD;
			} else if (age < HUMAN_YOUNG_MIDDLEAGE_BORDER) {
				return SPEED_FEMALE_YOUNG;
			} else if (age < HUMAN_MIDDLEAGE_ELDERLY_BORDER) {
				return SPEED_FEMALE_MIDDLEAGE;
			} else {
				return SPEED_FEMALE_ELDERLY;
			}
		} else {
			if (age < HUMAN_CHILD_YOUNG_BORDER) {
				return SPEED_MALE_CHILD;
			} else if (age < HUMAN_YOUNG_MIDDLEAGE_BORDER) {
				return SPEED_MALE_YOUNG;
			} else if (age < HUMAN_MIDDLEAGE_ELDERLY_BORDER) {
				return SPEED_MALE_MIDDLEAGE;
			} else {
				return SPEED_MALE_ELDERLY;
			}
		}
	}
}

double getDeathRate(LivingEntity * living, simClock currentTime) {
	int age = currentTime - living->wasBorn;
	if (living->gender == FEMALE) {
		if (age < HUMAN_CHILD_YOUNG_BORDER) {
			return PROBABILITY_FEMALE_CHILD_DEATH;
		} else if (age < HUMAN_YOUNG_MIDDLEAGE_BORDER) {
			return PROBABILITY_FEMALE_YOUNG_DEATH;
		} else if (age < HUMAN_MIDDLEAGE_ELDERLY_BORDER) {
			return PROBABILITY_FEMALE_MIDDLEAGE_DEATH;
		} else {
			return PROBABILITY_FEMALE_ELDERLY_DEATH;
		}
	} else {
		if (age < HUMAN_CHILD_YOUNG_BORDER) {
			return PROBABILITY_MALE_CHILD_DEATH;
		} else if (age < HUMAN_YOUNG_MIDDLEAGE_BORDER) {
			return PROBABILITY_MALE_YOUNG_DEATH;
		} else if (age < HUMAN_MIDDLEAGE_ELDERLY_BORDER) {
			return PROBABILITY_MALE_MIDDLEAGE_DEATH;
		} else {
			return PROBABILITY_MALE_ELDERLY_DEATH;
		}
	}
}

double getDecompositionRate(Zombie * zombie, simClock currentTime) {
	int age = currentTime - zombie->becameZombie;
	if (age < ZOMBIE_YOUNG_OLD_BORDER) {
		return PROBABILITY_ZOMBIE_YOUNG_DEATH;
	} else {
		return PROBABILITY_ZOMBIE_OLD_DEATH;
	}
}
