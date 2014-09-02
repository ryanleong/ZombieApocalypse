#include <stdbool.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "simulation.h"
#include "common.h"
#include "bitarray.h"

void lock(int x, int around, BitArray * locks) {
#ifdef _OPENMP
	for (bool locked = false; locked == false; /*NOP*/) {
#pragma omp critical (LockRegion)
		{
			for (int i = MAX(0, x - around); i < MIN(x + around, locks->size - 1);
					i++) {
				locked = locked && !testBit(locks, i);
			}
			if (locked) {
				for (int i = MAX(0, x - around);
						i < MIN(x + around, locks->size - 1); i++) {
					setBit(locks, i);
				}
			}
		}
	}
#endif
}

void unlock(int x, int around, BitArray * locks) {
#ifdef _OPENMP
#pragma omp critical (LockRegion)
	{
		for (int i = MAX(0, x - around); i < MIN(x + around, locks->size - 1);
				i++) {
			clearBit(locks, i);
		}
	}
#endif
}

#ifdef _OPENMP
bool numberOfThreads(int width) {
	int threads = omp_get_max_threads();
	return MIN(MAX(width / 3, 1), threads);
}
#endif

BitArray * locks = NULL;

void simulateStep(World * input, World * output) {
	if (locks == NULL) {
		locks = newBitArray(input->width);
	}

	output->clock = input->clock + 1;

#ifdef _OPENMP
#pragma omp parallel for default(shared) num_threads(numberOfThreads(input->width))
#endif
	for (int i = 1; i <= input->width; i++) {
		lock(i, 1, locks);
		for (int j = 1; j <= input->height; j++) {
			Tile * in = &input->map[i][j];
			Tile * out = &output->map[i][j];

			// TODO continue here by specifying rules
		}
		unlock(i, 1, locks);
	}
}
