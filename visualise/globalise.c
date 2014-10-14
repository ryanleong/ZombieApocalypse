#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <string.h>
#include <stdbool.h>

#include "../apocalypse/stats.h"

typedef enum type {
	IMAGE, STATS
} type;

FILE *** intoMatrix(glob_t * globbuf, type type, int * width, int * height) {
	int maxX = 0;
	int maxY = 0;
	for (int i = 0; i < globbuf->gl_pathc; i++) {
		char path[256];
		int t;
		int x;
		int y;
		if (type == IMAGE) {
			sscanf(globbuf->gl_pathv[i], "%[^0-9-]-%d-%d-%d.img", path, &t, &x,
					&y);
		} else if (type == STATS) {
			sscanf(globbuf->gl_pathv[i], "%[^0-9-]-%d-%d.out", path, &x, &y);
		} else {
			exit(3);
		}
		if (x > maxX) {
			maxX = x;
		}
		if (y > maxY) {
			maxY = y;
		}
	}

	*width = maxX + 1;
	*height = maxY + 1;

	FILE *** matrix = (FILE ***) malloc(sizeof(FILE **) * (*width));
	for (int i = 0; i < *width; i++) {
		matrix[i] = (FILE **) malloc(sizeof(FILE *) * (*height));
	}

	for (int i = 0; i < globbuf->gl_pathc; i++) {
		char path[256];
		int t;
		int x;
		int y;
		if (type == IMAGE) {
			sscanf(globbuf->gl_pathv[i], "%[^0-9-]-%d-%d-%d.img", path, &t, &x,
					&y);
		} else if (type == STATS) {
			sscanf(globbuf->gl_pathv[i], "%[^0-9-]-%d-%d.out", path, &x, &y);
		} else {
			exit(3);
		}
		FILE * file = fopen(globbuf->gl_pathv[i], "r");
		matrix[x][y] = file;
	}

	return matrix;
}

void closeMatrix(FILE *** matrix, int width, int height) {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			fclose(matrix[x][y]);
		}
		free(matrix[x]);
	}
	free(matrix);
}

void globaliseImage(FILE * out, FILE *** matrix, int width, int height) {
	int globalWidth = 0;
	int globalHeight = 0;
	long long int globalTime = 0;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int w;
			int h;
			long long int time;
			fscanf(matrix[x][y], "Width %d; Height %d; Time %lld\n", &w, &h,
					&time);

			globalTime = time;
			if (x == 0) {
				globalHeight += h;
			}
			if (y == 0) {
				globalWidth += w;
			}
			fseek(matrix[x][y], 0, SEEK_SET);
		}
	}

	fprintf(out, "Width %d; Height %d; Time %lld\n", globalWidth, globalHeight,
			globalTime);

	int offsetX = 0;
	for (int x = 0; x < width; x++) {
		int offsetY = 0;
		int w;

		for (int y = 0; y < height; y++) {
			int h;
			long long int time;
			fscanf(matrix[x][y], "Width %d; Height %d; Time %lld\n", &w, &h,
					&time);
			do {
				int xx;
				int yy;
				char type;
				char gender;
				int age;
				int whatsGoingOn = fscanf(matrix[x][y], "[%d %d] %c %c %d\n",
						&xx, &yy, &type, &gender, &age);
				if (whatsGoingOn == EOF) {
					break;
				}
				fprintf(out, "[%d %d] %c %c %d\n", xx + offsetX, yy + offsetY,
						type, gender, age);

			} while (1);

			offsetY += h;
		}
		offsetX += w;
	}
}

void globaliseStats(FILE * out, FILE *** matrix, int width, int height) {
	bool detailed = false;
	bool formatKnown = false;

	do {
		long long int time;
		int humans = 0;
		int infected = 0;
		int zombies = 0;

		Stats stats = NO_STATS;
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				char line[1024];

				int h, i, z;
				char * whatsGoingOn;
				whatsGoingOn = fgets(line, sizeof(line), matrix[x][y]);
				if (whatsGoingOn == NULL) {
					return;
				}

				sscanf(line,
						"Time: %6lld \tHumans: %6d \tInfected: %6d \tZombies: %6d\n",
						&time, &h, &i, &z);
				humans += h;
				infected += i;
				zombies += z;

				if (!formatKnown) {
					int pos = ftell(matrix[x][y]);
					fgets(line, sizeof(line), matrix[x][y]);
					detailed = (line[0] == 'L');
					formatKnown = true;
					fseek(matrix[x][y], pos, SEEK_SET);
				}

				if (detailed) {
					fgets(line, sizeof(line), matrix[x][y]);
					int lhf, lhm, lif, lim, lz;
					sscanf(line,
							"LHF: %6d \tLHM: %6d \tLIF: %6d \tLIM: %6d \tLZ:  %6d\n",
							&lhf, &lhm, &lif, &lim, &lz);
					stats.humanFemales += lhf;
					stats.humanMales += lhm;
					stats.infectedFemales += lif;
					stats.infectedMales += lim;
					stats.zombies += lz;

					fgets(line, sizeof(line), matrix[x][y]);
					int dhf, dhm, dif, dim, dz;
					sscanf(line,
							"DHF: %6d \tDHM: %6d \tDIF: %6d \tDIM: %6d \tDZ:  %6d\n",
							&dhf, &dhm, &dif, &dim, &dz);
					stats.humanFemalesDied += dhf;
					stats.humanMalesDied += dhm;
					stats.infectedFemalesDied += dif;
					stats.infectedMalesDied += dim;
					stats.zombiesDecomposed += dz;

					fgets(line, sizeof(line), matrix[x][y]);
					int bhf, bhm, bif, bim;
					sscanf(line, "BHF: %6d \tBHM: %6d \tBIF: %6d \tBIM: %6d\n",
							&bhf, &bhm, &bif, &bim);
					stats.humanFemalesBorn += bhf;
					stats.humanMalesBorn += bhm;
					stats.infectedFemalesBorn += bif;
					stats.infectedMalesBorn += bim;

					fgets(line, sizeof(line), matrix[x][y]);
					int ph, pi, gbh, gbi, cml, cc;
					sscanf(line,
							"PH:  %6d \tPI:  %6d \tGBH: %6d \tGBI: %6d \tCML: %6d \tCC:  %6d\n",
							&ph, &pi, &gbh, &gbi, &cml, &cc);
					stats.humanFemalesPregnant += ph;
					stats.infectedFemalesPregnant += pi;
					stats.humanFemalesGivingBirth += gbh;
					stats.infectedFemalesGivingBirth += gbi;
					stats.couplesMakingLove += cml;
					stats.childrenConceived += cc;

					fgets(line, sizeof(line), matrix[x][y]);
					int ihf, ihm, ifz, imz;
					sscanf(line, "IHF: %6d \tIHM: %6d \tIFZ: %6d \tIMZ: %6d\n",
							&ihf, &ihm, &ifz, &imz);
					stats.humanFemalesBecameInfected += ihf;
					stats.humanMalesBecameInfected += ihm;
					stats.infectedFemalesBecameZombies += ifz;
					stats.infectedMalesBecameZombies += imz;
				}
			}
		}

		// the same format as in ../apocalypse/output.c
		fprintf(out,
				"Time: %6lld \tHumans: %6d \tInfected: %6d \tZombies: %6d\n",
				time, humans, infected, zombies);

		if (detailed) {
			fprintf(out,
					"LHF: %6d \tLHM: %6d \tLIF: %6d \tLIM: %6d \tLZ:  %6d\n",
					stats.humanFemales, stats.humanMales, stats.infectedFemales,
					stats.infectedMales, stats.zombies);
			fprintf(out,
					"DHF: %6d \tDHM: %6d \tDIF: %6d \tDIM: %6d \tDZ:  %6d\n",
					stats.humanFemalesDied, stats.humanMalesDied,
					stats.infectedFemalesDied, stats.infectedMalesDied,
					stats.zombiesDecomposed);
			fprintf(out, "BHF: %6d \tBHM: %6d \tBIF: %6d \tBIM: %6d\n",
					stats.humanFemalesBorn, stats.humanMalesBorn,
					stats.infectedFemalesBorn, stats.infectedMalesBorn);
			fprintf(out,
					"PH:  %6d \tPI:  %6d \tGBH: %6d \tGBI: %6d \tCML: %6d \tCC:  %6d\n",
					stats.humanFemalesPregnant, stats.infectedFemalesPregnant,
					stats.humanFemalesBecameInfected,
					stats.infectedFemalesGivingBirth, stats.couplesMakingLove,
					stats.childrenConceived);
			fprintf(out, "IHF: %6d \tIHM: %6d \tIFZ: %6d \tIMZ: %6d\n",
					stats.humanFemalesBecameInfected,
					stats.humanMalesBecameInfected,
					stats.infectedFemalesBecameZombies,
					stats.infectedMalesBecameZombies);
		}
	} while (1);
}

int main(int argc, char ** argv) {
	if (argc != 2 && argc != 3) {
		printf("I want input file and possibly output file.\n");
		exit(1);
	}

	char path[256];
	int t;
	int x;
	int y;

	glob_t globbuf;
	char template[256];

	int width;
	int height;

	FILE * out = NULL;
	FILE *** matrix = NULL;

	int matched = sscanf(argv[1], "%[^0-9-]-%d-%d-%d.img", path, &t, &x, &y);
	if (matched == 4) {
		sprintf(template, "%s-%06d-*-*.img", path, t);
		glob(template, 0, NULL, &globbuf);

		matrix = intoMatrix(&globbuf, IMAGE, &width, &height);

		if (argc == 2) {
			char output[256];
			sprintf(output, "%s-%06d.img", path, t);
			out = fopen(output, "w");
		} else {
			out = fopen(argv[2], "w");
		}

		globaliseImage(out, matrix, width, height);
	} else {
		matched = sscanf(argv[1], "%[^0-9-]-%d-%d.out", path, &x, &y);
		if (matched == 3) {
			sprintf(template, "%s-*-*.out", path);
			glob(template, 0, NULL, &globbuf);

			matrix = intoMatrix(&globbuf, STATS, &width, &height);

			if (argc == 2) {
				char output[256];
				sprintf(output, "%s.out", path);
				out = fopen(output, "w");
			} else {
				out = fopen(argv[2], "w");
			}

			globaliseStats(out, matrix, width, height);
		} else {
			fprintf(stderr, "I don't know what to do with file %s\n", argv[1]);
			exit(2);

		}
	}

	fclose(out);
	closeMatrix(matrix, width, height);
	globfree(&globbuf);
}
