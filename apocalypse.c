#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <png.h>

#include "world.h"
#include "entity.h"
#include "random.h"
#include "simulation.h"
#include "common.h"
#include "constants.h"

void randomDistribution(World * w, int people, int zombies, simClock clock) {
	for (int i = 0; i < people;) {
		int x = randomInt(w->xStart, w->xEnd);
		int y = randomInt(w->yStart, w->yEnd);
		Tile * tile = GET_TILE(w, x, y);
		if (tile->entity != NULL) {
			continue;
		}

		Human * human = newHuman(clock);
		tile->entity = human->asEntity;

		i++;
	}

	for (int i = 0; i < zombies;) {
		int x = randomInt(w->xStart, w->xEnd);
		int y = randomInt(w->yStart, w->yEnd);
		Tile * tile = GET_TILE(w, x, y);
		if (tile->entity != NULL) {
			continue;
		}

		Zombie * zombie = newZombie(clock);
		tile->entity = zombie->asEntity;

		i++;
	}
}

void setRGB(png_byte *ptr, Tile * tile) {
	if (tile->entity == NULL) {
		ptr[0] = 255;
		ptr[1] = 255;
		ptr[2] = 255;
		return;
	}

	switch (tile->entity->type) {
	case HUMAN: {
		Human * h = tile->entity->asHuman;
		ptr[0] = 0;
		if (h->gender == FEMALE) {
			ptr[1] = 200;
		} else {
			ptr[1] = 150;
		}
		ptr[2] = 0;
		break;
	}
	case INFECTED: {
		Infected * i = tile->entity->asInfected;
		ptr[0] = 0;
		ptr[1] = 0;
		if (i->gender == FEMALE) {
			ptr[2] = 200;
		} else {
			ptr[2] = 150;
		}
		break;
	}
	case ZOMBIE: {
		//Zombie * z = tile->entity->asZombie;
		ptr[0] = 200;
		ptr[1] = 0;
		ptr[2] = 0;
		break;
	}
	}
}

int printWorld(World * world) {
	char filename[80];
	sprintf(filename, "images/step-%06lld.png", world->clock);

	int code = 0;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep row;

	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, world->width, world->height, 8,
	PNG_COLOR_TYPE_RGB,
	PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
	PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel - RGB)
	row = (png_bytep) malloc(3 * world->width * sizeof(png_byte));

	// Write image data
	for (int y = world->yStart; y <= world->yEnd; y++) {
		for (int x = world->xStart; x <= world->xEnd; x++) {
			setRGB(&(row[(x - world->xStart) * 3]), GET_TILE(world, x, y));
		}
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	finalise: if (fp != NULL)
		fclose(fp);
	if (info_ptr != NULL)
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL)
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	if (row != NULL)
		free(row);

	return code;
}

/**
 *  Print the number of humans, infected people (who carry the disease, but
 *  haven't yet become zombies), and zombies, for debugging.
 */
void printPopulations(World * world) {
	int humans = 0, infected = 0, zombies = 0;

	for (int x = world->xStart; x <= world->xEnd; x++) {
		for (int y = world->yStart; y <= world->yEnd; y++) {
			Tile *currentCell = GET_TILE(world, x, y);

			// ignore cells that are not occupied. A cell is only occupied
			// if the entity pointer is not null.
			if (currentCell->entity == NULL)
				continue;

			switch (currentCell->entity->type) {
			case HUMAN:
				humans++;
				break;

			case INFECTED:
				infected++;
				break;

			case ZOMBIE:
				zombies++;
				break;
			}
		}
	}

	printf("Time: %6d   Humans: %4d, Infected: %4d, Zombies: %4d.\n",
			(int) world->clock, humans, infected, zombies);
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
	printWorld(input);

	for (int i = 0; i < iters; i++) {
		simulateStep(input, output);
		finishStep(input, output);
		printWorld(output);
		printPopulations(output);

		World * temp = input;
		input = output;
		output = temp;
	}
}
