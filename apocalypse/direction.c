/*
 * direction.c
 *
 *  Created on: Sep 15, 2014
 *      Author: adam
 */

#include <math.h>

#include "direction.h"
#include "random.h"

bearing getRandomBearing() {
	double angle = randomDouble() * 2 * M_PI;
	return 0.5 * cos(angle) + 0.5 * I * sin(angle);
}

Direction bearingToDirection(bearing bearing) {
	if (cabs(bearing) < 1) {
		return STAY;
	}
	double angle = carg(bearing); // in range [-PI, +PI]

	// note that negative imaginary part means UP
	if (angle >= M_PI * (-3) / 4 && angle < M_PI * (-1) / 4) {
		return UP;
	} else if (angle >= M_PI * (-1) / 4 && angle < M_PI * 1 / 4) {
		return RIGHT;
	} else if (angle >= M_PI * 1 / 4 && angle < M_PI * 3 / 4) {
		return DOWN;
	} else {
		return LEFT;
	}
}
