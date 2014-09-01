#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "world.h"
#include "random.h"
#include "simulation.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("I want width and height.\n");
		exit(1);
	}

	unsigned int width = atoi(argv[1]);
	unsigned int height = atoi(argv[2]);

	initRandom(0);

	World * input = newWorld(width, height);
	World * output = newWorld(width, height);

	while (true) {
		simulateStep(input, output);

		World * temp = input;
		input = output;
		output = temp;
	}
}
