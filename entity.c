#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

#include "entity.h"
#include "common.h"
#include "random.h"
#include "constants.h"

/**
 * Returns random number of children.
 */
static int randomCountOfUnborn() {
	double rnd = randomDouble();
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
static void newChildren(Entity * entity, simClock clock, bool already) {
	int count = randomCountOfUnborn();
	entity->children = count;
	simClock event = randomEvent(PREGNANCY_DURATION_MEAN,
	PREGNANCY_DURATION_STD_DEV);
	if (already) {
		double rate = randomDouble();
		entity->borns = MAX(1, clock + (1 - rate) * event - entity->origin); // in future!
	} else {
		entity->borns = MAX(1, clock + event - entity->origin);
	}
}

void newHuman(Entity * human, simClock clock) {
	human->type = HUMAN;

	double rnd = randomDouble();
	simClock fertilityStart;
	simClock fertilityEnd;
	if (rnd < FEMALE_MALE_RATIO) {
		human->gender = FEMALE;

		if (randomDouble() < PROBABILITY_INITIAL_PREGNANCY) {
			newChildren(human, clock, true);
		} else {
			human->children = 0;
			human->borns = 0;
		}

		fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
		FERTILITY_START_FEMALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
		FERTILITY_END_FEMALE_STD_DEV);
	} else {
		human->gender = MALE;
		human->children = 0;
		human->borns = 0;

		fertilityStart = randomEvent(FERTILITY_START_MALE_MEAN,
		FERTILITY_START_MALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_MALE_MEAN,
		FERTILITY_END_MALE_STD_DEV);
	}

	double ageClass = randomDouble();
	double withinClass = randomDouble();

	if (ageClass < HUMAN_CHILD_POPULATION_SIZE) {
		human->origin = clock - (withinClass * HUMAN_CHILD_PERIOD);
	} else if (ageClass
			< HUMAN_YOUNG_POPULATION_SIZE + HUMAN_CHILD_POPULATION_SIZE) {
		human->origin = clock - HUMAN_CHILD_YOUNG_BORDER
				- withinClass * HUMAN_YOUNG_PERIOD;
	} else if (ageClass
			< HUMAN_MIDDLEAGE_POPULATION_SIZE + HUMAN_YOUNG_POPULATION_SIZE
					+ HUMAN_CHILD_POPULATION_SIZE) {
		human->origin = clock - HUMAN_YOUNG_MIDDLEAGE_BORDER
				- randomDouble() * HUMAN_MIDDLEAGE_PERIOD;
	} else {
		human->origin = clock - HUMAN_MIDDLEAGE_ELDERLY_BORDER
				- randomDouble() * HUMAN_ELDERLY_PERIOD;
	}

	// may or may not be in future
	human->fertilityStart = MAX(1, fertilityStart);
	human->fertilityEnd = MAX(fertilityStart, fertilityEnd);

	human->bearing = getRandomBearing();
	human->becameInfected = 0;
}

void newZombie(Entity * zombie, simClock clock) {
	zombie->type = ZOMBIE;
	zombie->origin = clock; // right now
	zombie->bearing = getRandomBearing();

	zombie->children = 0;
	zombie->borns = 0;
	zombie->fertilityStart = 0;
	zombie->fertilityEnd = 0;
	zombie->becameInfected = 0;
}

void toInfected(Entity * infected, simClock clock) {
	infected->type = INFECTED;
	infected->becameInfected = clock - infected->origin;
}

void toZombie(Entity * zombie, simClock clock) {
	zombie->type = ZOMBIE;
	zombie->origin = clock;
	// after becoming zombie, the bearing is restarted
	zombie->bearing = getRandomBearing();

	zombie->children = 0;
	zombie->borns = 0;
	zombie->fertilityStart = 0;
	zombie->fertilityEnd = 0;
	zombie->becameInfected = 0;
}

void makeLove(Entity * mother, Entity * father, simClock clock, Stats stats) {
	int children = stats.humanFemalesDied + stats.humanMalesDied;
	int couples = stats.couplesMakingLove;
	if (couples == 0) {
		return;
	}

	double rnd = randomDouble();
	if (stats.humanFemales + stats.humanMales > 12000) {
		if (rnd > children / (double) couples) {
			return;
		}
	} else {
		if (rnd > 10 * PROBABILITY_FERTILIZATION) { //0.006 human stable
			return;
		}
	}
	newChildren(mother, clock, false);
}

Entity giveBirth(Entity * mother, simClock clock) {
	Entity born;
	born.type = NONE;

	if (mother->children <= 0) {
		return born;
	}

	switch (mother->type) {
	case HUMAN: {
		born.type = HUMAN;
		born.becameInfected = 0;
		break;
	}
	case INFECTED: {
		born.type = INFECTED;
		born.becameInfected = 0;
		break;
	}
	default: // won't happen, silent GCC
		break;
	}

	double rnd = randomDouble();
	simClock fertilityStart;
	simClock fertilityEnd;
	if (rnd < FEMALE_MALE_RATIO) {
		born.gender = FEMALE;
		fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
		FERTILITY_START_FEMALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
		FERTILITY_END_FEMALE_STD_DEV);
	} else {
		born.gender = MALE;
		fertilityStart = randomEvent(FERTILITY_START_MALE_MEAN,
		FERTILITY_START_MALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_MALE_MEAN,
		FERTILITY_END_MALE_STD_DEV);
	}

	born.origin = clock;
	born.fertilityStart = fertilityStart;
	born.fertilityEnd = fertilityEnd;

	born.bearing = NO_BEARING;
	born.children = 0;
	born.borns = 0;

	// decrease number of unborn children
	mother->children--;
	return born;
}

double getMaxSpeed(Entity * entity, simClock currentTime) {
	if (entity->type == ZOMBIE) {
		int age = currentTime - entity->origin;
		if (age < ZOMBIE_YOUNG_OLD_BORDER) {
			return SPEED_ZOMBIE_YOUNG;
		} else {
			return SPEED_ZOMBIE_OLD;
		}
	} else {
		int age = currentTime - entity->origin;
		if (entity->gender == FEMALE) {
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

double getDeathRate(Entity * living, simClock currentTime) {
	int age = currentTime - living->origin;
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

double getDecompositionRate(Entity * zombie, simClock currentTime) {
	int age = currentTime - zombie->origin;
	if (age < ZOMBIE_YOUNG_OLD_BORDER) {
		return PROBABILITY_ZOMBIE_YOUNG_DEATH;
	} else {
		return PROBABILITY_ZOMBIE_OLD_DEATH;
	}
}
