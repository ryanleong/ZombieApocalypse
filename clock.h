#ifndef CLOCK_H_
#define CLOCK_H_

/**
 * Type for time in simulation, the basic unit is defined as 1 day.
 */
typedef long long int simClock;

/**
 * The basic unit is defined as 1 day.
 */
#define CLOCK_UNITS_IN_DAY 1

/**
 * How many units the time interval contains
 */
#define IN_DAYS CLOCK_UNITS_IN_DAY
#define IN_WEEKS (CLOCK_UNITS_IN_DAY * 7)
#define IN_MONTHS (CLOCK_UNITS_IN_DAY * 30)
#define IN_YEARS (CLOCK_UNITS_IN_DAY * 365)

#endif /* CLOCK_H_ */
