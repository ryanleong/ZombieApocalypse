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
#include "common.h"

#ifndef OUTPUT_EVERY
#define OUTPUT_EVERY 1
#endif


/**
 *  Set up initial populations of humans and zombies, with random starting
 *  locations.
 */
    static void
init_world (World *world, int num_people, int num_zombies)
{
    int x, y;

    for (int i = 0; i < num_people; )
    {
        // choose random coordinates.
        x = randomInt (0, world->width - 1);
        y = randomInt (0, world->height - 1);

        // check if there is already a person at the selected location.
        if (world->map [y] [x].entity_type != EMPTY)
            continue;

        // the location now has a person there.
        new_human (& (world->map [y] [x].entity.human));
        world->map [y] [x].entity_type = HUMAN;

        // keep looping until we have created the initial human population.
        i ++;
    }

    // same process for creating the initial zombie population.
    for (int i = 0; i < num_zombies; )
    {
        x = randomInt (0, world->width - 1);
        y = randomInt (0, world->height - 1);

        if (world->map [y] [x].entity_type != EMPTY)
            continue;

        new_zombie (& (world->map [y] [x].entity.zombie));
        world->map [y] [x].entity_type = ZOMBIE;
        i ++;
    }
}

void setRGB(png_byte *ptr, Tile * tile) {
	switch (tile->entity_type) 
    {
	case HUMAN:
		ptr[0] = 0;
		ptr[1] = 255;
		ptr[2] = 0;
		break;

	case INFECTED:
		ptr[0] = 0;
		ptr[1] = 0;
		if (tile->entity.infected.gender == FEMALE) {
			ptr[2] = 200;
		} else {
			ptr[2] = 150;
		}
		break;

	case ZOMBIE:
		ptr[0] = 255;
		ptr[1] = 0;
		ptr[2] = 0;
		break;

    case EMPTY:
		ptr[0] = 255;
		ptr[1] = 255;
		ptr[2] = 255;
        break;
	}
}

/**
 * Generates an image for the world mapping each tile to a pixel.
 */
int printWorld(World * world) {
	char filename[80];
	sprintf(filename, "images/step-%06lld.png", world->clock);

	int code = 0;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;

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
	png_bytep * image = (png_bytep*) malloc(sizeof(png_bytep) * world->height);
	for (int i = 0; i < world->height; i++) {
		image[i] = (png_bytep) malloc(3 * world->width * sizeof(png_byte));
	}

	// Prepare image data; this can be done in parallel
#ifdef _OPENMP
#pragma omp parallel for collapse(2) schedule(guided, 10)
#endif
	for (int y = world->yStart; y <= world->yEnd; y++) {
		for (int x = world->xStart; x <= world->xEnd; x++) {
			setRGB(image[y - world->yStart] + (x - world->xStart) * 3,
					& (world->map [x] [y]));
		}
	}
	png_write_image(png_ptr, image);

	// End write
	png_write_end(png_ptr, NULL);

	finalise: if (fp != NULL)
		fclose(fp);
	if (info_ptr != NULL)
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL)
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	if (image != NULL) {
		for (int i = 0; i < world->height; i++) {
			free(image[i]);
		}
		free(image);
	}

	return code;
}

/**
 *  Print the number of humans, infected people (who carry the disease, but
 *  haven't yet become zombies), and zombies, for debugging.
 */
void printPopulations(World * world) {
	int humans = 0, infected = 0, zombies = 0;

#ifdef _OPENMP
#pragma omp parallel for collapse(2) schedule(guided, 10) reduction(+: humans, infected, zombies)
#endif
	for (int i = 0; i < world->height; i++) {
		for (int j = 0; j < world->width; j++) {
			switch (world->map [i] [j].entity_type) {
			case HUMAN:
				humans++;
				break;

			case INFECTED:
				infected++;
				break;

			case ZOMBIE:
				zombies++;
				break;

            case EMPTY:
                // empty cell. Ignore.
                break;
			}
		}
	}

	// make sure there are always blanks around numbers
	// that way we can easily split the line
	printf("Time: %6lld \tHumans: %4d \tInfected: %4d \tZombies: %4d\n",
			world->clock, humans, infected, zombies);
}

void printStatistics(World * world) {
	if (world->clock % OUTPUT_EVERY > 0) {
		return;
	}

#ifndef NPOPULATION
	printPopulations(world);
#endif

#ifndef NIMAGES
	printWorld(world);
#endif
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

	init_world (input, people, zombies);
	printWorld(input);

	for (int i = 0; i < iters; i++) {
		simulation_step (input, output);
		finishStep(input, output);
		printStatistics(output);

		World * temp = input;
		input = output;
		output = temp;
	}

    return 0;
}
