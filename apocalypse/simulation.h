#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "world.h"

/**
 * Runs one step of the world simulation.
 * The implementation should use the input map only for reading.
 */
void simulateStep(WorldPtr input, WorldPtr output);

#endif /* SIMULATION_H_ */
