#include <stdbool.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "simulation.h"
#include "common.h"
#include "random.h"

void simulateStep(World * input, World * output) {
	output->clock = input->clock + 1;

#ifdef _OPENMP
	int threads = omp_get_max_threads();
	int numThreads = MIN(MAX(input->width / 3, 1), threads);
	// at least three columns per thread
#pragma omp parallel for default(shared) num_threads(numThreads)
#endif
	for (int x = 0; x < input->width; x++) {
		for (int y = 0; y < input->height; y++) {
			Tile * in = getTile(input, x, y);

			// just an example of random movement

			for (LivingEntity * le = in->living; le != NULL; le =
					le->nextLiving) {
				int rnd = randomInt(0, 4);
				Tile * t = NULL;
				if (rnd != 0) {
					switch (rnd) {
					case 1:
						t = getTile(output, x + 1, y);
						break;
					case 2:
						t = getTile(output, x, y + 1);
						break;
					case 3:
						t = getTile(output, x - 1, y);
						break;
					case 4:
						t = getTile(output, x, y - 1);
						break;
					}
				}
				if (t == NULL) {
					t = getTile(output, x, y);
				}
				lockTile(t);
				LivingEntity * le_ = copyLiving(le);
				le_->nextLiving = t->living;
				t->living = le_;
				unlockTile(t);
			}

			for (Zombie * z = in->zombies; z != NULL; z = z->nextZombie) {
				int rnd = randomInt(0, 4);
				Tile * t = NULL;
				if (rnd != 0) {
					switch (rnd) {
					case 1:
						t = getTile(output, x + 1, y);
						break;
					case 2:
						t = getTile(output, x, y + 1);
						break;
					case 3:
						t = getTile(output, x - 1, y);
						break;
					case 4:
						t = getTile(output, x, y - 1);
						break;
					}
				}
				if (t == NULL) {
					t = getTile(output, x, y);
				}
				lockTile(t);
				Zombie * z_ = copyZombie(z);
				z_->nextZombie = t->zombies;
				t->zombies = z_;
				unlockTile(t);
			}

			// TODO continue here by specifying rules
		}
	}
}
