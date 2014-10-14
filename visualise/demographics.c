#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../apocalypse/clock.h"
#include "../apocalypse/common.h"

#define MAX_AGE_YEARS 100

typedef enum EntityTypes {
	HUMAN_MALE,
	HUMAN_FEMALE,
	HUMAN_PREGNANT,
	INFECTED_MALE,
	INFECTED_FEMALE,
	INFECTED_PREGNANT,
	ZOMBIE,
	ENTITY_TYPES_COUNT
} EntityTypes;

void printDemographics(FILE * in, FILE * out) {
	int width;
	int height;
	int time;

	int demographics[MAX_AGE_YEARS + 1][ENTITY_TYPES_COUNT] = { { 0 } };

	fscanf(in, "Width %d; Height %d; Time %d\n", &width, &height, &time);

	do {
		int x;
		int y;
		char type;
		char gender;
		int age;
		int whatsGoingOn = fscanf(in, "[%d %d] %c %c %d\n", &x, &y, &type,
				&gender, &age);
		if (whatsGoingOn == EOF) {
			break;
		}

		int years = MIN(age / (type=='z' ? IN_MONTHS : IN_YEARS),
				MAX_AGE_YEARS);

		switch (type) {
		case 'H':
			switch (gender) {
			case 'M':
				demographics[years][HUMAN_MALE]++;
				break;
			case 'f':
				demographics[years][HUMAN_PREGNANT]++;
				// no break
			case 'F':
				demographics[years][HUMAN_FEMALE]++;
				break;
			}
			break;
		case 'I':
			switch (gender) {
			case 'M':
				demographics[years][INFECTED_MALE]++;
				break;
			case 'f':
				demographics[years][INFECTED_PREGNANT]++;
				// no break
			case 'F':
				demographics[years][INFECTED_FEMALE]++;
				break;
			}
			break;
		case 'Z':
			demographics[years][ZOMBIE]++;
		}
	} while (1);

	for (int i = 0; i <= MAX_AGE_YEARS; i++) {
		fprintf(out, "Age: %d "
				"HM: %d HF: %d HP: %d "
				"IM: %d IF: %d IP: %d "
				"Z: %d\n", i, demographics[i][HUMAN_MALE],
				demographics[i][HUMAN_FEMALE], demographics[i][HUMAN_PREGNANT],
				demographics[i][INFECTED_MALE],
				demographics[i][INFECTED_FEMALE],
				demographics[i][INFECTED_PREGNANT], demographics[i][ZOMBIE]);
	}
}

int main(int argc, char ** argv) {
	if (argc != 2 && argc != 3) {
		printf("I want input file and possibly output file.\n");
		exit(1);
	}

	FILE * in = fopen(argv[1], "r");

	char * outName;
	if (argc == 3) {
		outName = argv[2];
	} else {
		outName = argv[1];
		int len = strlen(outName);
		outName[len - 3] = 'd';
		outName[len - 2] = 'e';
		outName[len - 1] = 'm';
	}

	FILE * out = fopen(outName, "w");

	printDemographics(in, out);

	fclose(out);
	fclose(in);

	exit(0);
}
