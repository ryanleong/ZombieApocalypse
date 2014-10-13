#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include "world.h"

void sendRecieveBorder(WorldPtr world);

void sendRecieveBorderFinish(WorldPtr world);

void sendReceiveGhosts(WorldPtr world);

void sendReceiveGhostsFinish(WorldPtr World);

double divideWorld(int * width, int * height, WorldPtr * input,
		WorldPtr * output);

#endif /* COMMUNICATION_H_ */
