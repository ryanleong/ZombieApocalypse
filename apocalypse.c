#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef TIME
#include <sys/time.h>
#endif

#ifndef NIMAGES
#include <png.h>
#endif

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
 * Fills the world with specified number of people and zombies.
 * The people are of different age; zombies are "brand new".
 */
void randomDistribution(World * w, int people, int zombies, simClock clock) {
	for (int i = 0; i < people;) {
		int x = randomInt(w->xStart, w->xEnd);
		int y = randomInt(w->yStart, w->yEnd);
		Entity * entityPtr = &GET_ENTITY(w, x, y);
		if (entityPtr->type != NONE) {
			continue;
		}

		newHuman(entityPtr, clock);
		if (entityPtr->gender == FEMALE) {
			w->lastStats.humanFemales++;
		} else {
			w->lastStats.humanMales++;
		}

		i++;
	}

	for (int i = 0; i < zombies;) {
		int x = randomInt(w->xStart, w->xEnd);
		int y = randomInt(w->yStart, w->yEnd);
		Entity * entityPtr = &GET_ENTITY(w, x, y);
		if (entityPtr->type != NONE) {
			continue;
		}

		newZombie(entityPtr, clock);
		w->lastStats.zombies++;

		i++;
	}
}

#ifndef NIMAGES

/**
 * Fills an image pixel with a color based on properties of the tile and entity.
 * Humans are green, infected are blue and zombies are red.
 */
void setRGB(png_byte *ptr, Entity * entity, simClock clock) {
	// TODO make the color depend on age; this is low priority
	// I tried it but the difference was not noticeable

	switch (entity->type) {
	case HUMAN: {
		if (entity->children > 0) {
			ptr[0] = 100;
		} else {
			ptr[0] = 0;
		}
		if (entity->gender == FEMALE) {
			ptr[1] = 200;
		} else {
			ptr[1] = 150;
		}
		ptr[2] = 0;
		break;
	}
	case INFECTED: {
		if (entity->children > 0) {
			ptr[0] = 100;
		} else {
			ptr[0] = 0;
		}
		ptr[1] = 0;
		if (entity->gender == FEMALE) {
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
	case NONE:
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
					&GET_ENTITY(world, x, y), world->clock);
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
#endif

/**
 *  Print the number of humans, infected people (who carry the disease, but
 *  haven't yet become zombies), and zombies, for debugging.
 */
void printPopulations(World * world) {
	Stats s = world->stats;
	// make sure there are always blanks around numbers
	// that way we can easily split the line
	printf("Time: %6lld \tHumans: %6d \tInfected: %6d \tZombies: %6d\n",
			world->clock, s.humanFemales + s.humanMales,
			s.infectedFemales + s.infectedMales, s.zombies);

#ifndef NDETAILED_STATS
	printf("LHF: %6d \tLHM: %6d \tLIF: %6d \tLIM: %6d \tLZ:  %6d\n",
			s.humanFemales, s.humanMales, s.infectedFemales, s.infectedMales,
			s.zombies);
	printf("DHF: %6d \tDHM: %6d \tDIF: %6d \tDIM: %6d \tDZ:  %6d\n",
			s.humanFemalesDied, s.humanMalesDied, s.infectedFemalesDied,
			s.infectedMalesDied, s.zombiesDecomposed);
	printf("BHF: %6d \tBHM: %6d \tBIF: %6d \tBIM: %6d\n", s.humanFemalesBorn,
			s.humanMalesBorn, s.infectedFemalesBorn, s.infectedMalesBorn);
	printf("PH:  %6d \tPI:  %6d \tGBH: %6d \tGBI: %6d \tCML: %6d \tCC:  %6d\n",
			s.humanFemalesPregnant, s.infectedFemalesPregnant,
			s.humanFemalesBecameInfected, s.infectedFemalesGivingBirth,
			s.couplesMakingLove, s.childrenConceived);
	printf("IHF: %6d \tIHM: %6d \tIFZ: %6d \tIMZ: %6d\n",
			s.humanFemalesBecameInfected, s.humanMalesBecameInfected,
			s.infectedFemalesBecameZombies, s.infectedMalesBecameZombies);
#endif
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
	if (argc != 5) {
		printf("I want width, height, zombies, iterations.\n");
		exit(1);
	}

	unsigned int width = atoi(argv[1]);
	unsigned int height = atoi(argv[2]);

	unsigned int people = (int) (width * height * INITIAL_DENSITY);
	unsigned int zombies = atoi(argv[3]);

	unsigned int iters = atoi(argv[4]);

	initRandom(0);

	World * input = newWorld(width, height);
	World * output = newWorld(width, height);

	randomDistribution(input, people, zombies, 0);
#ifndef NIMAGES
	printWorld(input);
#endif

#ifdef TIME
	struct timeval t1, t2;
	double elapsedTime;
	gettimeofday(&t1, NULL);
#endif

	for (int i = 0; i < iters; i++) {
		simulateStep(input, output);
		finishStep(input, output);
		printStatistics(output);

		Stats stats = output->stats;
		World * temp = input;
		input = output;
		output = temp;
		input->lastStats = stats;
		resetWorld(output);
		copyStats(output, stats);
	}

#ifdef TIME
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; // sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;// us to ms

#ifdef _OPENMP
	int numThreads = omp_get_max_threads();
#else
	int numThreads = 1;
#endif
	printf("Took %f milliseconds with %d threads\n", elapsedTime, numThreads);
#endif

// this is a clean up
// we destroy both worlds
	destroyWorld(input);
	destroyWorld(output);

	destroyRandom();
}
