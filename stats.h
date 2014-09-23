#ifndef STATS_H_
#define STATS_H_

typedef struct Stats {
    int humanFemales;
    int humanMales;
    int infectedFemales;
    int infectedMales;
    int zombies;
    int humanFemalesDied;
    int humanMalesDied;
    int infectedFemalesDied;
    int infectedMalesDied;
    int zombiesDecomposed;
    int humanFemalesBorn;
    int humanMalesBorn;
    int humanFemalesGivingBirth;
    int humanFemalesPregnant;
    int infectedFemalesBorn;
    int infectedMalesBorn;
    int infectedFemalesGivingBirth;
    int infectedFemalesPregnant;
    int couplesMakingLove;
    int childrenConceived;
    int humanFemalesBecameInfected;
    int humanMalesBecameInfected;
    int infectedFemalesBecameZombies;
    int infectedMalesBecameZombies;
} Stats;

#define NO_STATS ((const struct Stats) {0})

#endif /* STATS_H_ */

// vim: ts=4 sw=4 et
