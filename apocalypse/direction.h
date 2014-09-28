/*
 * direction.h
 *
 *  Created on: Sep 15, 2014
 *      Author: adam
 */

#ifndef DIRECTION_H_
#define DIRECTION_H_

#include <complex.h>

/**
 * Mapping names to indices of directions.
 * Direction 0 is STAY.
 * Directions 1-4 are the adjacent cells.
 * Directions 5-12 are cells accessible within two moves.
 * There are helper constants DIRECTION_START, DIRECTION_BASIC and DIRECTION_ALL.
 * Use them if you want to iterate over directions.
 */
typedef enum Direction {
	STAY, // distance = 0
	DIRECTION_START = 1,
	LEFT = 1, // distance = 1
	UP,
	RIGHT,
	DOWN,
	DIRECTION_BASIC = 4,
	LL, // distance = 2
	UL,
	UU,
	UR,
	RR,
	DR,
	DD,
	DL,
	DIRECTION_ALL = 12
} Direction;

#define RANDOM_BASIC_DIRECTIONS 24

static const int random_basic_directions[][4] = { { LEFT, UP, RIGHT, DOWN }, {
		LEFT, UP, DOWN, RIGHT }, { LEFT, RIGHT, UP, DOWN }, { LEFT, RIGHT, DOWN,
		UP }, { LEFT, DOWN, UP, RIGHT }, { LEFT, DOWN, RIGHT, UP }, { UP, LEFT,
		RIGHT, DOWN }, { UP, LEFT, DOWN, RIGHT }, { UP, RIGHT, LEFT, DOWN }, {
		UP, RIGHT, DOWN, LEFT }, { UP, DOWN, LEFT, RIGHT }, { UP, DOWN, RIGHT,
		LEFT }, { RIGHT, LEFT, UP, DOWN }, { RIGHT, LEFT, DOWN, UP }, { RIGHT,
		UP, LEFT, DOWN }, { RIGHT, UP, DOWN, LEFT }, { RIGHT, DOWN, LEFT, UP },
		{ RIGHT, DOWN, UP, LEFT }, { DOWN, LEFT, UP, RIGHT }, { DOWN, LEFT,
				RIGHT, UP }, { DOWN, UP, LEFT, RIGHT },
		{ DOWN, UP, RIGHT, LEFT }, { DOWN, RIGHT, LEFT, UP }, { DOWN, RIGHT, UP,
				LEFT } };

/**
 * Mapping direction to movement in x.
 */
static const int direction_delta_x[] = { 0, -1, 0, 1, 0, -2, -1, 0, 1, 2, 1, 0,
		-1 };

/**
 * Mapping direction to movement in y.
 */
static const int direction_delta_y[] = { 0, 0, -1, 0, 1, 0, -1, -2, -1, 0, 1, 2,
		1 };

/**
 * Rotates the direction counter-clock-wise.
 * Works only for BASIC directions.
 */
#define DIRECTION_CCW(dir) ((dir + 2) % 4 + 1)

/**
 * Rotates the direction clock-wise.
 * Works only for BASIC directions.
 */
#define DIRECTION_CW(dir) ((dir) % 4 + 1)

/**
 * The bearing is simply a complex number.
 * Note that 0+1i means DOWN.
 */
typedef float complex bearing;

/**
 * Constant which means that there is no bearing.
 * Another bearing can be added with normal arithmetics.
 */
#define NO_BEARING ((const bearing) (0+0*I))

/**
 * Returns random bearing in terms of random angle and absolute value = 0.5.
 * This is intentional to map any random bearing to
 * direction STAY in function bearingToDirection.
 */
bearing getRandomBearing();

/**
 * Normalizes the bearing, strips its absolute value.
 */
#define BEARING_PROJECT(bearing) ((bearing) / cabs(bearing))

/**
 * Converts a direction to bearing.
 * This function is useful when iterating over directions.
 */
#define BEARING_FROM_DIRECTION(direction) \
	(direction_delta_x[direction] + I * direction_delta_y[direction])

/**
 * Converts bearing to direction.
 * The plane is divided into 5 parts.
 * In the middle, the circle with radius = 1 is considered STAY.
 * The rest is divided as the letter X - LEFT, UP, RIGHT and DOWN.
 */
Direction bearingToDirection(bearing bearing);

#endif /* DIRECTION_H_ */
