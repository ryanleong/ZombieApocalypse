#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "world.h"

/**
 * Runs one step of the world simulation.
 * The implementation should use the input map only for reading.
 */
void simulation_step (const World * input, World * output);

/**
 * Does the cleaning after each step of simulation.
 * This means:
 * 1) moving entities on BORDER back to REGULAR tiles.
 * 2) resetting the world.
 */
void finishStep(World * input, World * output);

/**
 * Returns optimal number of threads which to use for step simulating.
 * This number is also necessary for allocators.
 */
int getNumThreads(int width);

#endif /* SIMULATION_H_ */
