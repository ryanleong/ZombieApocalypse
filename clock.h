#ifndef CLOCK_H_
#define CLOCK_H_

typedef unsigned long long int simClock;
typedef unsigned long long int simClock_days;
typedef unsigned long long int simClock_weeks;
typedef unsigned long long int simClock_months;
typedef unsigned long long int simClock_years;

#define CLOCK_UNITS_IN_DAY 24;

#define toDays(clock) ((clock) / 24)
#define toWeeks(clock) ((clock) / (24 * 7))
#define toMonths(clock)((clock) / (24 * 30))
#define toYears(clock) ((clock) / (24 * 265))

#endif /* CLOCK_H_ */
