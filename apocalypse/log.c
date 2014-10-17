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

Timer startTimer() {
	Timer timer;
	gettimeofday(&timer, NULL);
	return timer;
}

double getElapsedTime(Timer start) {
	Timer end;
	gettimeofday(&end, NULL);

	double elapsedTime;
	elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
	elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0; // us to ms

	return elapsedTime;

}
