#include <stdio.h>
#include <stdbool.h>

#include "output.h"
#include "log.h"

void printWorld(WorldPtr world, bool borders) {
	char gender[5] = { 'M', 'F', 'f', 'f', 'f' };

	char filename[255];
	if (world->globalColumns == 1 && world->globalRows == 1) {
		sprintf(filename, "images/step-%06lld.img", world->clock);
	} else {
		sprintf(filename, "images/step-%06lld-%d-%d.img", world->clock,
				world->globalX, world->globalY);
	}

	FILE * out = fopen(filename, "w");
	if (out == NULL) {
		LOG_ERROR("Could not open file %s for writing\n", filename);
		return;
	}

	fprintf(out, "Width %d; Height %d; Time %lld\n",
			world->localWidth + (borders ? 4 : 0),
			world->localHeight + (borders ? 4 : 0), world->clock);

	for (int y = 0; y < world->localHeight + (borders ? 4 : 0); y++) {
		for (int x = 0; x < world->localWidth + (borders ? 4 : 0); x++) {
			CellPtr ptr = GET_CELL_PTR(world, x + (borders ? 0 : world->xStart),
					y + (borders ? 0 : world->yStart));
			int age = world->clock - ptr->origin;
			switch (ptr->type) {
			case NONE:
				// nothing
				break;
			case HUMAN:
				fprintf(out, "[%d %d] H %c %d\n", x, y,
						gender[ptr->gender + ptr->children], age);
				break;
			case INFECTED:
				fprintf(out, "[%d %d] I %c %d\n", x, y,
						gender[ptr->gender + ptr->children], age);
				break;
			case ZOMBIE:
				fprintf(out, "[%d %d] Z _ %d\n", x, y, age);
			}
		}
	}

	fclose(out);
}

void printPopulations(Stats stats) {
	// make sure there are always blanks around numbers
	// that way we can easily split the line
	printf("Time: %6lld \tHumans: %6d \tInfected: %6d \tZombies: %6d\n",
			stats.clock, stats.humanFemales + stats.humanMales,
			stats.infectedFemales + stats.infectedMales, stats.zombies);

#ifndef NDETAILED_STATS
	printf("LHF: %6d \tLHM: %6d \tLIF: %6d \tLIM: %6d \tLZ:  %6d\n",
			stats.humanFemales, stats.humanMales, stats.infectedFemales,
			stats.infectedMales, stats.zombies);
	printf("DHF: %6d \tDHM: %6d \tDIF: %6d \tDIM: %6d \tDZ:  %6d\n",
			stats.humanFemalesDied, stats.humanMalesDied,
			stats.infectedFemalesDied, stats.infectedMalesDied,
			stats.zombiesDecomposed);
	printf("BHF: %6d \tBHM: %6d \tBIF: %6d \tBIM: %6d\n",
			stats.humanFemalesBorn, stats.humanMalesBorn,
			stats.infectedFemalesBorn, stats.infectedMalesBorn);
	printf("PH:  %6d \tPI:  %6d \tGBH: %6d \tGBI: %6d \tCML: %6d \tCC:  %6d\n",
			stats.humanFemalesPregnant, stats.infectedFemalesPregnant,
			stats.humanFemalesBecameInfected, stats.infectedFemalesGivingBirth,
			stats.couplesMakingLove, stats.childrenConceived);
	printf("IHF: %6d \tIHM: %6d \tIFZ: %6d \tIMZ: %6d\n",
			stats.humanFemalesBecameInfected, stats.humanMalesBecameInfected,
			stats.infectedFemalesBecameZombies,
			stats.infectedMalesBecameZombies);
#endif
}

void printStatistics(WorldPtr world, Stats cumulative) {
#ifndef NIMAGES
	if (world->clock % IMAGES_EVERY == 0) {
		printWorld(world, false);
	}
#endif

#ifndef NPOPULATION
	if (world->clock % POPULATION_EVERY == 0) {
#ifndef NCUMULATIVE_STATS
		printPopulations(cumulative);
#else
		printPopulations(world->stats);
#endif
	}
#endif
}
