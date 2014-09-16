/*
 * direction.h
 *
 *  Created on: Sep 15, 2014
 *      Author: adam
 */

#ifndef DIRECTION_H_
#define DIRECTION_H_

#include <complex.h>

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

static const int direction_delta_x[] = { 0, -1, 0, 1, 0, -2, -1, 0, 1, 2, 1, 0,
		-1 };
static const int direction_delta_y[] = { 0, 0, -1, 0, 1, 0, -1, -2, -1, 0, 1, 2,
		1 };

#define DIRECTION_CCW(dir) ((dir + 2) % 4 + 1)
#define DIRECTION_CW(dir) ((dir) % 4 + 1)

typedef double complex bearing;

#define NO_BEARING ((const double complex) (0+0*I))

bearing getRandomBearing();

#define BEARING_ADD(base, bearing, relevance) \
	base += ((bearing) * (relevance))

#define BEARING_PROJECT(bearing) ((bearing) / cabs(bearing))

#define BEARING_FROM_DIRECTION(direction) \
	(direction_delta_x[direction] + I * direction_delta_y[direction])

Direction bearingToDirection(bearing bearing);

#endif /* DIRECTION_H_ */
