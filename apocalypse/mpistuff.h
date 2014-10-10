#ifndef MPISTUFF_H_
#define MPISTUFF_H_

#define DIMENSIONS 2

typedef int coords[DIMENSIONS];

#ifdef USE_MPI

#include <mpi.h>

#endif

#endif /* MPISTUFF_H_ */
