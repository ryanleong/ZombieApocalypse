/*
 * constants.h
 *
 *  Created on: Sep 1, 2014
 *      Author: adam
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include "clock.h"

// #### TESTING ####
// these defines are necessary for report

//#define UNCONTROLLED_BIRTH 1
//#define EQUAL_BIRTH 1
//#define DENSITY_BIRTH 1

// humans are divided into 4 classes: CHILD, YOUNG, MIDDLEAGE, ELDERLY//
// zombies have only two classes: YOUNG and OLD
// age dependent constants must contain:
// name of the class and either gender or entity type

// #### AGING ####

// ## AGE CLASSES ##

// these defines are deprecated and will be removed in the future
//#define LIFE_EXPECTANCY_MALE_MEAN (81 * IN_YEARS) // wiki
//#define LIFE_EXPECTANCY_FEMALE_MEAN (85 * IN_YEARS) // wiki
#define ZOMBIE_DECOMPOSITION_MEAN (3 * IN_YEARS) // 3 to 5 years
// end of deprecated defines

#define HUMAN_CHILD_PERIOD (15 * IN_YEARS)
#define HUMAN_CHILD_POPULATION_SIZE 0.182
#define HUMAN_CHILD_YOUNG_BORDER (HUMAN_CHILD_PERIOD)

#define HUMAN_YOUNG_PERIOD (22 * IN_YEARS + 6 * IN_MONTHS)
#define HUMAN_YOUNG_POPULATION_SIZE 0.318
#define HUMAN_YOUNG_MIDDLEAGE_BORDER (HUMAN_CHILD_YOUNG_BORDER + HUMAN_YOUNG_PERIOD)

#define HUMAN_MIDDLEAGE_PERIOD (27 * IN_YEARS + 6 * IN_MONTHS)
#define HUMAN_MIDDLEAGE_POPULATION_SIZE 0.356
#define HUMAN_MIDDLEAGE_ELDERLY_BORDER (HUMAN_YOUNG_MIDDLEAGE_BORDER + HUMAN_MIDDLEAGE_PERIOD)

#define HUMAN_ELDERLY_PERIOD (45 * IN_YEARS)
#define HUMAN_ELDERLY_POPULATION_SIZE 0.144

#define ZOMBIE_YOUNG_PERIOD (18 * IN_MONTHS)
#define ZOMBIE_YOUNG_OLD_BORDER (ZOMBIE_YOUNG_PERIOD)

// ## HUMAN DEATH PROBABILITIES ##

// difference between lifespan of males and females
#define GENDER_DEATH_DIFF 1.024539666710089 // sqrt(82.4 / 78.5)

#define PROBABILITY_MALE_CHILD_DEATH (0.0000006 * GENDER_DEATH_DIFF)
#define PROBABILITY_FEMALE_CHILD_DEATH (0.0000006 / GENDER_DEATH_DIFF)

#define PROBABILITY_MALE_YOUNG_DEATH (0.0000012 * GENDER_DEATH_DIFF)
#define PROBABILITY_FEMALE_YOUNG_DEATH (0.0000012 / GENDER_DEATH_DIFF)

#define PROBABILITY_MALE_MIDDLEAGE_DEATH (0.0000096 * GENDER_DEATH_DIFF)
#define PROBABILITY_FEMALE_MIDDLEAGE_DEATH (0.0000096 / GENDER_DEATH_DIFF)

#define PROBABILITY_MALE_ELDERLY_DEATH (0.000096 * GENDER_DEATH_DIFF)
#define PROBABILITY_FEMALE_ELDERLY_DEATH (0.000096 / GENDER_DEATH_DIFF)

// ## ZOMBIE AND INFECTED RELATED PROBABILITIES

#define PROBABILITY_ZOMBIE_YOUNG_DEATH (1.0 / ZOMBIE_DECOMPOSITION_MEAN)
#define PROBABILITY_ZOMBIE_OLD_DEATH (1.0 / ZOMBIE_DECOMPOSITION_MEAN)

#define ZOMBIE_INFECTION_MEAN (14 * IN_DAYS)
#define PROBABILITY_BECOME_ZOMBIE (1.0 / ZOMBIE_INFECTION_MEAN)

// #### BIRTH ####

// ## NUMBER OF CHILDREN ##

// http://en.wikipedia.org/wiki/Multiple_birth#Causes_and_frequency
// 1:89 (= 1.1%) or 1:80 (= 1.25%) for twins
// 1:892 (= 1:7921, about 0.013%) or 1:802 (= 1:6400) for triplets

#define PROBABILITY_TWO_CHILDREN (0.011)
#define PROBABILITY_THREE_CHILDEN (0.00013)
#define PROBABILITY_ONE_CHILD (1 - PROBABILITY_TWO_CHILDREN - PROBABILITY_THREE_CHILDEN)

// ## PREGNANCY DURATION ##
#define PREGNANCY_DURATION_MEAN (9 * IN_MONTHS)
#define PREGNANCY_DURATION_STD_DEV (2 * IN_WEEKS)

// ## FERTILITY PERIOD ##
#define FERTILITY_START_FEMALE_MEAN (15 * IN_YEARS)
#define FERTILITY_START_FEMALE_STD_DEV (2 * IN_YEARS)
#define FERTILITY_START_MALE_MEAN (15 * IN_YEARS)
#define FERTILITY_START_MALE_STD_DEV (2 * IN_YEARS)

#define FERTILITY_END_FEMALE_MEAN (45 * IN_YEARS)
#define FERTILITY_END_FEMALE_STD_DEV (5 * IN_YEARS)
#define FERTILITY_END_MALE_MEAN (80 * IN_YEARS)
#define FERTILITY_END_MALE_STD_DEV (10 * IN_YEARS)

// #### MOVEMENT ####

// ## HUMANS ##

// Base of ratio of male to female avg speed (0.85714311076)
#define FEMALE_TO_MALE_SPEED_RATIO 0.85714311076

#define SPEED_MALE_CHILD 0.64
#define SPEED_FEMALE_CHILD (SPEED_MALE_CHILD * FEMALE_TO_MALE_SPEED_RATIO)

#define SPEED_MALE_YOUNG 0.8
#define SPEED_FEMALE_YOUNG (SPEED_MALE_YOUNG * FEMALE_TO_MALE_SPEED_RATIO)

#define SPEED_MALE_MIDDLEAGE 0.48
#define SPEED_FEMALE_MIDDLEAGE (SPEED_MALE_MIDDLEAGE * FEMALE_TO_MALE_SPEED_RATIO)

#define SPEED_MALE_ELDERLY 0.4
#define SPEED_FEMALE_ELDERLY (SPEED_MALE_ELDERLY * FEMALE_TO_MALE_SPEED_RATIO)

// ## ZOMBIES ##

#define SPEED_ZOMBIE_YOUNG 0.4
#define SPEED_ZOMBIE_OLD (0.4 * 0.8)

// #### DIRECTION ####

// ## CELLS RATING ##

// TODO review these constants

#define BEARING_RATE_LIVING_SAME_SEX_ONE (1)
#define BEARING_RATE_LIVING_OPPOSITE_SEX_ONE (6)
#define BEARING_RATE_LIVING_ZOMBIE_ONE (-4)
#define BEARING_RATE_LIVING_EMPTY_ONE (1)

#define BEARING_RATE_LIVING_SAME_SEX_TWO (0.2)
#define BEARING_RATE_LIVING_OPPOSITE_SEX_TWO (4)
#define BEARING_RATE_LIVING_ZOMBIE_TWO (-3)
#define BEARING_RATE_LIVING_EMPTY_TWO (1)

#define BEARING_RATE_ZOMBIE_LIVING_ONE (5)
#define BEARING_RATE_ZOMBIE_ZOMBIE_ONE (1)
#define BEARING_RATE_ZOMBIE_EMPTY_ONE (1.5)

#define BEARING_RATE_ZOMBIE_LIVING_TWO (4)
#define BEARING_RATE_ZOMBIE_ZOMBIE_TWO (1)
#define BEARING_RATE_ZOMBIE_EMPTY_TWO (1.5)

// ## RANDOM FLUCTUATION ##

#define BEARING_FLUCTUATION 0.35
#define BEARING_ABS_QUOTIENT_MEAN 1
#define BEARING_ABS_QUOTIENT_VARIANCE 0.25

// ## FOLLOWING DIRECTION ##

#define DIRECTION_FOLLOW 0.65
#define DIRECTION_MISSED 0.1

// probability that an entity will try alternative
// if the first choice is not possible
#define MOVEMENT_TRY_ALTERNATIVE 0.3

// #### OTHER PROBABILITIES ####

// probability of becoming infected when a zombie as at the adjacent cell
#define PROBABILITY_INFECTION 0.0075

// probability of conceiving children during love making
#define PROBABILITY_FERTILIZATION 0.00073

// probability of an entity being born as a female
#define FEMALE_MALE_RATIO_WHEN_BORN 0.4854

// probability of an entity being a female when the simulation stats
#define FEMALE_MALE_RATIO_INITIAL 0.498

// per whole life
#define CHILDREN_PER_FEMALE 1.77

// probability of a female being pregnant when the simulation starts
#define PROBABILITY_INITIAL_PREGNANCY \
		(CHILDREN_PER_FEMALE * PREGNANCY_DURATION_MEAN) \
		/ (FERTILITY_END_FEMALE_MEAN - FERTILITY_START_FEMALE_MEAN)

#define SITUATION_AWARENESS_COEFFICIENT 2.5

// population density in NT
#ifdef POPULATION_8000
#define INITIAL_DENSITY 0.032
#else
#define INITIAL_DENSITY 0.085
#endif

#endif /* CONSTANTS_H_ */
