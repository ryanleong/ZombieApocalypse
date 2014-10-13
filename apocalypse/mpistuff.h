#ifndef MPISTUFF_H_
#define MPISTUFF_H_

#define DIMENSIONS 2

typedef int coords[DIMENSIONS];

//#define USE_MPI

#ifdef USE_MPI

#include <mpi.h>

// 4 input sides: each side is two sends and two receives
// 4 output sides: each side is one send and one receive
// 1 output stats: 1 gather all
#define MAX_REQUESTS (4*4)

#define SHIFT_LEFT_RIGHT 1
#define SHIFT_UP_DOWN 1

// tags containing numeral 1 are the more distant ones
#define TOP_INPUT_BORDER_TAG 0
#define TOP1_INPUT_BORDER_TAG 1
#define BOTTOM_INPUT_BORDER_TAG 2
#define BOTTOM1_INPUT_BORDER_TAG 3
#define LEFT_INPUT_BORDER_TAG 4
#define LEFT1_INPUT_BORDER_TAG 5
#define RIGHT_INPUT_BORDER_TAG 6
#define RIGHT1_INPUT_BORDER_TAG 7

#define TOP_OUTPUT_BORDER_TAG 8
#define BOTTOM_OUTPUT_BORDER_TAG 9
#define LEFT_OUTPUT_BORDER_TAG 10
#define RIGHT_OUTPUT_BORDER_TAG 11

#endif

#endif /* MPISTUFF_H_ */
