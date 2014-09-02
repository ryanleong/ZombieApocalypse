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
	for (int i = 0; i < people; i++) {
		int x = randomInt(0, w->width - 1);
		int y = randomInt(0, w->height - 1);

		Human * human = newHuman(clock);
		Tile * tile = getTile(w, x, y);
		human->nextLiving = tile->living;
		tile->living = human->asLiving;
	}

	for (int i = 0; i < zombies; i++) {
		int x = randomInt(0, w->width - 1);
		int y = randomInt(0, w->height - 1);

		Zombie * zombie = newZombie(clock);
		Tile * tile = getTile(w, x, y);
		zombie->nextZombie = tile->zombies;
		tile->zombies = zombie;
	}
}

void setRGB(png_byte *ptr, Tile * tile) {
	int humans = 0;
	int infected = 0;
	for (LivingEntity * e = tile->living; e != NULL; e = e->nextLiving) {
		switch (e->type) {
		case HUMAN:
			humans++;
			break;
		case INFECTED:
			infected++;
			break;
		default: // won't happen
			break;
		}
	}

	int zombies = 0;
	for (LivingEntity * e = tile->zombies; e != NULL; e = e->nextZombie) {
		zombies++;
	}

	ptr[0] = MIN(zombies * 255 / MAX_COLOR, 255); // red = zombie
	ptr[1] = MIN(humans * 255 / MAX_COLOR, 255); // green = human
	ptr[2] = MIN(infected * 255 / MAX_COLOR, 255); // blue = infected
}

void printWorld(World * world) {
	char filename[80];
	sprintf(filename, "images/step-%06d.png", world->clock);

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
	for (int y = 0; y < world->height; y++) {
		for (int x = 0; x < world->width; x++) {
			setRGB(&(row[x * 3]), getTile(world, x, y));
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

	// return code;
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
		//printWorld(output);

		World * temp = input;
		input = output;
		output = temp;
		resetWorld(output);
	}
}
