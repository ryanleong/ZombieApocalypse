#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>
#include <sys/time.h>

// change to NULL to suppress the logging
#define LOG_DEBUG_OUTPUT stderr
#define LOG_ERROR_OUTPUT stderr

#define LOG_EVENT_OUTPUT NULL
#define LOG_TIME_OUTPUT stderr
#define LOG_POPULATION_OUTPUT stdout

#define LOG_UNIVERSAL(output, ...) \
	({ if (output != NULL) {fprintf(output, __VA_ARGS__);} })

#define LOG_DEBUG(...) \
	LOG_UNIVERSAL(LOG_DEBUG_OUTPUT, "DEBUG: " __VA_ARGS__)

#define LOG_ERROR(...) \
	LOG_UNIVERSAL(LOG_ERROR_OUTPUT, "ERROR: " __VA_ARGS__)

#define LOG_EVENT(...) \
	LOG_UNIVERSAL(LOG_EVENT_OUTPUT, "EVENT: " __VA_ARGS__)

#define LOG_TIME(...) \
	LOG_UNIVERSAL(LOG_TIME_OUTPUT, "TIME: " __VA_ARGS__)

#define LOG_POPULATION(...) \
	LOG_UNIVERSAL(LOG_POPULATION_OUTPUT, "" __VA_ARGS__)

void initRedirectToFiles();

void finishRedirectToFiles();

typedef struct timeval Timer;

Timer startTimer();

double getCurrentTimeDiff(Timer start);

#endif
