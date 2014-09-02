#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "world.h"
#include "entity.h"
#include "random.h"
#include "simulation.h"

void randomDistribution(World * w, int people, int zombies, simClock clock) {
	for (int i = 0; i < people; i++) {
		int x = randomInt(0, w->width - 1);
		int y = randomInt(0, w->height - 1);

		Human * human = newHuman(clock);
		human->nextLiving = w->map[x][y].living;
		w->map[x][y].living = human->asLiving;
	}

	for (int i = 0; i < zombies; i++) {
		int x = randomInt(0, w->width - 1);
		int y = randomInt(0, w->height - 1);

		Zombie * zombie = newZombie(clock);
		zombie->nextZombie = w->map[x][y].zombies;
		w->map[x][y].zombies = zombie;
	}
}

void printWorld(World * w) {

}

int main(int argc, char **argv) {
	if (argc != 6) {
		printf("I want width, height, people, zombies, iterations.\n");
		exit(1);
	}

	unsigned int width = atoi(argv[1]);
	unsigned int height = atoi(argv[2]);
	unsigned int people = atoi(argv[3]);
	unsigned int zombies = atoi(argv[4]);

	unsigned int iters = atoi(argv[5]);

	initRandom(0);

	World * input = newWorld(width, height);
	World * output = newWorld(width, height);

	randomDistribution(input, people, zombies, 0);

	for (int i = 0; i < iters; i++) {
		simulateStep(input, output);
		printWorld(output);

		World * temp = input;
		input = output;
		output = temp;
	}
}
