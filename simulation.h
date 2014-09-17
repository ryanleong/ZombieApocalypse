#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "world.h"

/**
 * Runs one step of the world simulation.
 * The implementation should use the input map only for reading.
 *
 * This function will be called in every thread.
 */
void simulateStep(World * input, World * output);

/**
 * Does the cleaning after each step of simulation.
 * This means:
 * 1) moving entities on BORDER back to REGULAR tiles.
 * 2) resetting the world.
 *
 * This function will be called in every thread.
 */
void finishStep(World * input, World * output);

#endif /* SIMULATION_H_ */
