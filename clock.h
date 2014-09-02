#ifndef CLOCK_H_
#define CLOCK_H_

typedef long long int simClock;
typedef long long int simClock_days;
typedef long long int simClock_weeks;
typedef long long int simClock_months;
typedef long long int simClock_years;

#define CLOCK_UNITS_IN_DAY 24;

#define toDays(clock) ((clock) / 24)
#define toWeeks(clock) ((clock) / (24 * 7))
#define toMonths(clock)((clock) / (24 * 30))
#define toYears(clock) ((clock) / (24 * 365))
#define dayTime(clock) ((clock) % 24)

#define IN_DAYS * 24
#define IN_WEEKS * (24 * 7)
#define IN_MONTHS * (24 * 30)
#define IN_YEARS * (24 * 365)

#endif /* CLOCK_H_ */
