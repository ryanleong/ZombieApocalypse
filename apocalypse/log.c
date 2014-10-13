#include <stdio.h>

#include "log.h"
#include "world.h"

void initRedirectToFiles(WorldPtr world) {
	{
		char filenameOut[255];
		if (world->globalColumns == 1 && world->globalRows == 1) {
			sprintf(filenameOut, "output/apocalypse.out");
		} else {
			sprintf(filenameOut, "output/apocalypse-%d-%d.out", world->globalX,
					world->globalY);
		}

		freopen(filenameOut, "w", stdout);
	}

	{
		char filenameErr[255];
		if (world->globalColumns == 1 && world->globalRows == 1) {
			sprintf(filenameErr, "output/apocalypse.err");
		} else {
			sprintf(filenameErr, "output/apocalypse-%d-%d.err", world->globalX,
					world->globalY);
		}

		freopen(filenameErr, "w", stderr);
	}
}

void finishRedirectToFiles() {
	fclose(stdout);
	fclose(stderr);
}
