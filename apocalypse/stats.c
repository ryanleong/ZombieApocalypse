#include "stats.h"

void mergeStats(Stats * dest, Stats src, bool absolute) {
	if (absolute) {
		dest->humanFemales += src.humanFemales;
		dest->humanMales += src.humanMales;
		dest->infectedFemales += src.infectedFemales;
		dest->infectedMales += src.infectedMales;
		dest->zombies += src.zombies;
		dest->humanFemalesPregnant += src.humanFemalesPregnant;
		dest->infectedFemalesPregnant += src.infectedFemalesPregnant;
	} else {
		dest->humanFemales = src.humanFemales;
		dest->humanMales = src.humanMales;
		dest->infectedFemales = src.infectedFemales;
		dest->infectedMales = src.infectedMales;
		dest->zombies = src.zombies;
		dest->humanFemalesPregnant = src.humanFemalesPregnant;
		dest->infectedFemalesPregnant = src.infectedFemalesPregnant;
	}
	dest->humanFemalesDied += src.humanFemalesDied;
	dest->humanMalesDied += src.humanMalesDied;
	dest->infectedFemalesDied += src.infectedFemalesDied;
	dest->infectedMalesDied += src.infectedMalesDied;
	dest->zombiesDecomposed += src.zombiesDecomposed;
	dest->humanFemalesBorn += src.humanFemalesBorn;
	dest->humanMalesBorn += src.humanMalesBorn;
	dest->humanFemalesGivingBirth += src.humanFemalesGivingBirth;
	dest->infectedFemalesBorn += src.infectedFemalesBorn;
	dest->infectedMalesBorn += src.infectedMalesBorn;
	dest->infectedFemalesGivingBirth += src.infectedFemalesGivingBirth;
	dest->couplesMakingLove += src.couplesMakingLove;
	dest->childrenConceived += src.childrenConceived;
	dest->humanFemalesBecameInfected += src.humanFemalesBecameInfected;
	dest->humanMalesBecameInfected += src.humanMalesBecameInfected;
	dest->infectedFemalesBecameZombies += src.infectedFemalesBecameZombies;
	dest->infectedMalesBecameZombies += src.infectedMalesBecameZombies;
}
