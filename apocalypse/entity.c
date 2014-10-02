#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <math.h>

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
static void newChildren(EntityPtr mother, simClock clock, bool already) {
	int count = randomCountOfUnborn();
	mother->children = count;
	simClock event = randomEvent(PREGNANCY_DURATION_MEAN,
	PREGNANCY_DURATION_STD_DEV);
	if (already) {
		double rate = randomDouble();
		mother->borns = MAX(1, clock + (1 - rate) * event - mother->origin); // in future!
	} else {
		mother->borns = MAX(1, clock + event - mother->origin);
	}
}

void newHuman(EntityPtr human, simClock clock) {
	human->type = HUMAN;

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

	double rnd = randomDouble();
	simClock fertilityStart;
	simClock fertilityEnd;
	if (rnd < FEMALE_MALE_RATIO_INITIAL) {
		human->gender = FEMALE;

		fertilityStart = randomEvent(FERTILITY_START_FEMALE_MEAN,
		FERTILITY_START_FEMALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_FEMALE_MEAN,
		FERTILITY_END_FEMALE_STD_DEV);

		if ((clock > human->origin + fertilityStart
				&& clock < human->origin + fertilityEnd)
				&& randomDouble() < PROBABILITY_INITIAL_PREGNANCY) {
			newChildren(human, clock, true);
		} else {
			human->children = 0;
			human->borns = 0;
		}
	} else {
		human->gender = MALE;
		human->children = 0;
		human->borns = 0;

		fertilityStart = randomEvent(FERTILITY_START_MALE_MEAN,
		FERTILITY_START_MALE_STD_DEV);
		fertilityEnd = randomEvent(FERTILITY_END_MALE_MEAN,
		FERTILITY_END_MALE_STD_DEV);
	}

	// may or may not be in future
	human->fertilityStart = MAX(1, fertilityStart);
	human->fertilityEnd = MAX(fertilityStart, fertilityEnd);

	human->bearing = getRandomBearing();
	human->becameInfected = 0;
}

void newZombie(EntityPtr zombie, simClock clock) {
	zombie->type = ZOMBIE;
	zombie->origin = clock; // right now
	zombie->bearing = getRandomBearing();

	zombie->children = 0;
	zombie->borns = 0;
	zombie->fertilityStart = 0;
	zombie->fertilityEnd = 0;
	zombie->becameInfected = 0;
}

void toInfected(EntityPtr infected, simClock clock) {
	infected->type = INFECTED;
	infected->becameInfected = clock - infected->origin;
}

void toZombie(EntityPtr zombie, simClock clock) {
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

void makeLove(EntityPtr mother, EntityPtr father, simClock clock, Stats stats) {
	double rnd = randomDouble();

#ifdef UNCONTROLLED_BIRTH
	double prob = PROBABILITY_FERTILIZATION;
#elif EQUAL_BIRTH
	int died = stats.humanFemalesDied + stats.humanMalesDied
	+ stats.infectedFemalesDied + stats.infectedMalesDied
	+ stats.infectedFemalesBecameZombies
	+ stats.infectedMalesBecameZombies;
	int couples = stats.couplesMakingLove;
	double prob = couples == 0 ? 1 : died / couples;
#else // controlled by density
	int population = stats.humanFemales + stats.humanMales
			+ stats.infectedFemales + stats.infectedMales;
	double density = population / ((double) stats.width * stats.height);
	double ratio = INITIAL_DENSITY / density;
#ifdef DENSITY_BIRTH
	double q = ratio;
#else // controlled with power
	double exp = ratio * SITUATION_AWARENESS_COEFFICIENT;
	double q = pow(ratio, exp);
#endif
	double prob = PROBABILITY_FERTILIZATION * q;
#endif
	if (rnd > prob) {
		return;
	}

	newChildren(mother, clock, false);
}

Entity giveBirth(EntityPtr mother, simClock clock) {
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
	if (rnd < FEMALE_MALE_RATIO_WHEN_BORN) {
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

double getMaxSpeed(EntityPtr entity, simClock currentTime) {
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

double getDeathRate(EntityPtr living, simClock currentTime) {
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

double getDecompositionRate(EntityPtr zombie, simClock currentTime) {
	int age = currentTime - zombie->origin;
	if (age < ZOMBIE_YOUNG_OLD_BORDER) {
		return PROBABILITY_ZOMBIE_YOUNG_DEATH;
	} else {
		return PROBABILITY_ZOMBIE_OLD_DEATH;
	}
}
