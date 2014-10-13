#ifdef _OPENMP
#include <omp.h>
#endif

#include "communication.h"
#include "mpistuff.h"
#include "common.h"

void sendRecieveBorder(WorldPtr world) {
#ifdef USE_MPI
	int rank, destUp, destDown, destLeft, destRight;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Cart_shift(world->comm, SHIFT_UP_DOWN, -1, &rank, &destUp);
	MPI_Cart_shift(world->comm, SHIFT_UP_DOWN, +1, &rank, &destDown);
	MPI_Cart_shift(world->comm, SHIFT_LEFT_RIGHT, -1, &rank, &destLeft);
	MPI_Cart_shift(world->comm, SHIFT_LEFT_RIGHT, +1, &rank, &destRight);

	{
		// send top two rows
		CellPtr topRow = GET_CELL_PTR(world, world->xStart, world->yStart);
		MPI_Isend(topRow, 1, world->rowType, destUp, TOP_INPUT_BORDER_TAG,
				world->comm, world->requests + (world->requestCount++));
		CellPtr topRow1 = GET_CELL_PTR(world, world->xStart, world->yStart + 1);
		MPI_Isend(topRow1, 1, world->rowType, destUp, TOP1_INPUT_BORDER_TAG,
				world->comm, world->requests + (world->requestCount++));

		// receive top two rows -> bottom border
		CellPtr bottomBorder = GET_CELL_PTR(world, world->xStart,
				world->yEnd + 1);
		MPI_Irecv(bottomBorder, 1, world->rowType, destDown,
				TOP_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
		CellPtr bottomBorder1 = GET_CELL_PTR(world, world->xStart,
				world->yEnd + 2);
		MPI_Irecv(bottomBorder1, 1, world->rowType, destDown,
				TOP1_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}

	{
		// send bottom two rows
		CellPtr bottomRow = GET_CELL_PTR(world, world->xStart, world->yEnd);
		MPI_Isend(bottomRow, 1, world->rowType, destDown,
				BOTTOM_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
		CellPtr bottomRow1 = GET_CELL_PTR(world, world->xStart,
				world->yEnd - 1);
		MPI_Isend(bottomRow1, 1, world->rowType, destDown,
				BOTTOM1_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));

		// receive bottom two rows -> top border
		CellPtr topBorder = GET_CELL_PTR(world, world->xStart,
				world->yStart - 1);
		MPI_Irecv(topBorder, 1, world->rowType, destUp,
				BOTTOM_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
		CellPtr topBorder1 = GET_CELL_PTR(world, world->xStart,
				world->yStart - 2);
		MPI_Irecv(topBorder1, 1, world->rowType, destUp,
				BOTTOM1_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}

	{
		// send left two columns
		CellPtr leftColumn = GET_CELL_PTR(world, world->xStart, world->yStart);
		MPI_Isend(leftColumn, 1, world->columnType, destLeft,
				LEFT_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
		CellPtr leftRow1 = GET_CELL_PTR(world, world->xStart + 1,
				world->yStart);
		MPI_Isend(leftRow1, 1, world->columnType, destLeft,
				LEFT1_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));

		// receive left two columns -> right border
		CellPtr rightBorder = GET_CELL_PTR(world, world->xEnd + 1,
				world->yStart);
		MPI_Irecv(rightBorder, 1, world->columnType, destRight,
				LEFT_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
		CellPtr rightBorder1 = GET_CELL_PTR(world, world->xEnd + 2,
				world->yStart);
		MPI_Irecv(rightBorder1, 1, world->columnType, destRight,
				LEFT1_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}

	{
		// send right two columns
		CellPtr rightColumn = GET_CELL_PTR(world, world->xEnd, world->yStart);
		MPI_Isend(rightColumn, 1, world->columnType, destRight,
				RIGHT_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
		CellPtr rightColumn1 = GET_CELL_PTR(world, world->xEnd - 1,
				world->yStart);
		MPI_Isend(rightColumn1, 1, world->columnType, destRight,
				RIGHT1_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));

		// receive right two columns -> left border
		CellPtr leftBorder = GET_CELL_PTR(world, world->xStart - 1,
				world->yStart);
		MPI_Irecv(leftBorder, 1, world->columnType, destLeft,
				RIGHT_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
		CellPtr leftBorder1 = GET_CELL_PTR(world, world->xStart - 2,
				world->yStart);
		MPI_Irecv(leftBorder1, 1, world->columnType, destLeft,
				RIGHT1_INPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}
#else
	// just copy borders - periodic
	{
#ifdef _OPENMP
		int threads = omp_get_max_threads();
		int numThreads = MIN(MAX(world->localWidth / 10, 1), threads);
#pragma omp parallel for schedule(static) num_threads(numThreads)
#endif
		for (int x = world->xStart; x <= world->xEnd; x++) {
			// filling bottom border
			Cell x1 = GET_CELL(world, x, world->yStart);
			GET_CELL(world, x, world->yEnd + 1) = x1;
			Cell x2 = GET_CELL(world, x, world->yStart + 1);
			GET_CELL(world, x, world->yEnd + 2) = x2;

			// filling top border
			Cell x3 = GET_CELL(world, x, world->yEnd);
			GET_CELL(world, x, world->yStart - 1) = x3;
			Cell x4 = GET_CELL(world, x, world->yEnd - 1);
			GET_CELL(world, x, world->yStart - 2) = x4;
		}
	}
	{
#ifdef _OPENMP
		int threads = omp_get_max_threads();
		int numThreads = MIN(MAX(world->localHeight / 10, 1), threads);
#pragma omp parallel for schedule(static) num_threads(numThreads)
#endif
		for (int y = world->yStart; y <= world->yEnd; y++) {
			// filling right border
			Cell y1 = GET_CELL(world, world->xStart, y);
			GET_CELL(world, world->xEnd + 1, y) = y1;
			Cell y2 = GET_CELL(world, world->xStart + 1, y);
			GET_CELL(world, world->xEnd + 2, y) = y2;

			// filling left border
			Cell y3 = GET_CELL(world, world->xEnd, y);
			GET_CELL(world, world->xStart - 1, y) = y3;
			Cell y4 = GET_CELL(world, world->xEnd - 1, y);
			GET_CELL(world, world->xStart - 2, y) = y4;
		}
	}
#endif
}

void sendRecieveBorderFinish(WorldPtr world) {
#ifdef USE_MPI
	MPI_Waitall(world->requestCount, world->requests, MPI_STATUSES_IGNORE);
	world->requestCount = 0;
#endif
}

void sendReceiveGhosts(WorldPtr world) {
#ifdef USE_MPI
	int rank, destUp, destDown, destLeft, destRight;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Cart_shift(world->comm, SHIFT_UP_DOWN, -1, &rank, &destUp);
	MPI_Cart_shift(world->comm, SHIFT_UP_DOWN, +1, &rank, &destDown);
	MPI_Cart_shift(world->comm, SHIFT_LEFT_RIGHT, -1, &rank, &destLeft);
	MPI_Cart_shift(world->comm, SHIFT_LEFT_RIGHT, +1, &rank, &destRight);

	{
		// send top inner border
		CellPtr topInnerBorder = GET_CELL_PTR(world, world->xStart,
				world->yStart - 1);
		MPI_Isend(topInnerBorder, 1, world->rowType, destUp,
				TOP_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));

		// receive top inner border -> top outer border
		CellPtr bottomOuterBorder = GET_CELL_PTR(world, world->xStart,
				world->yEnd + 2);
		MPI_Irecv(bottomOuterBorder, 1, world->rowType, destDown,
				TOP_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}

	{
		// send bottom inner border
		CellPtr bottomInnerBorder = GET_CELL_PTR(world, world->xStart,
				world->yEnd + 1);
		MPI_Isend(bottomInnerBorder, 1, world->rowType, destDown,
				BOTTOM_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));

		// receive bottom inner border -> bottom outer border
		CellPtr topOuterBorder = GET_CELL_PTR(world, world->xStart,
				world->yStart - 2);
		MPI_Irecv(topOuterBorder, 1, world->rowType, destUp,
				BOTTOM_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}

	{
		// send left inner border
		CellPtr leftInnerBorder = GET_CELL_PTR(world, world->xStart - 1,
				world->yStart);
		MPI_Isend(leftInnerBorder, 1, world->columnType, destLeft,
				LEFT_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));

		// receive left inner border -> left outer border
		CellPtr rightOuterBorder = GET_CELL_PTR(world, world->xEnd + 2,
				world->yStart);
		MPI_Irecv(rightOuterBorder, 1, world->columnType, destRight,
				LEFT_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}

	{
		// send right inner border
		CellPtr rightInnerBorder = GET_CELL_PTR(world, world->xEnd + 1,
				world->yStart);
		MPI_Isend(rightInnerBorder, 1, world->columnType, destRight,
				RIGHT_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));

		// receive right inner border -> right outer border
		CellPtr leftOuterBorder = GET_CELL_PTR(world, world->xStart - 2,
				world->yStart);
		MPI_Irecv(leftOuterBorder, 1, world->columnType, destLeft,
				RIGHT_OUTPUT_BORDER_TAG, world->comm,
				world->requests + (world->requestCount++));
	}
#endif
}

/**
 * Moves back an entity which is on the ghost cell.
 */
static void mergeInto(WorldPtr world, int srcX, int srcY, int destX, int destY) {
	EntityPtr from = GET_CELL_PTR(world, srcX, srcY);
	if (from->type != NONE) {
		if (GET_CELL(world, destX, destY).type == NONE) {
			GET_CELL(world, destX, destY) = *from;
		}
		from->type = NONE;
	}
}

void sendReceiveGhostsFinish(WorldPtr world) {
#ifdef USE_MPI
	if (world->requestCount % 2 == 0) {
		MPI_Waitall(world->requestCount, world->requests, MPI_STATUSES_IGNORE);
		world->requestCount = 0;
	} else { // there must be one extra request, which is stats reducing
		// will wait for all except for the last request
		MPI_Waitall(world->requestCount-1, world->requests, MPI_STATUSES_IGNORE);
		world->requests[0] = world->requests[world->requestCount - 1];
		world->requestCount = 1;
	}
#endif

	// now, we will merge ghost into the last cell
	// either outer border cell if MPI is used; opposite inner border otherwise

	// we may leave border in any condition
	// next time the world will be input world
	// and the border will be replaced
	{
#ifdef _OPENMP
		int threads = omp_get_max_threads();
		int numThreads = MIN(MAX(world->localWidth / 10, 1), threads);
#pragma omp parallel for schedule(static) num_threads(numThreads)
#endif
		for (int x = world->xStart; x <= world->xEnd; x++) {
#ifdef USE_MPI
			int y1 = world->yStart - 2;
			int y2 = world->yEnd + 2;
#else
			int y1 = world->yEnd - 1;
			int y2 = world->yStart + 1;
#endif
			mergeInto(world, x, y1, x, world->yStart);
			mergeInto(world, x, y2, x, world->yEnd);
		}
	}

	{
#ifdef _OPENMP
		int threads = omp_get_max_threads();
		int numThreads = MIN(MAX(world->localHeight / 10, 1), threads);
#pragma omp parallel for schedule(static) num_threads(numThreads)
#endif
		for (int y = world->yStart; y <= world->yEnd; y++) {
#ifdef USE_MPI
			int x1 = world->xStart - 2;
			int x2 = world->xEnd + 2;
#else
			int x1 = world->xEnd - 1;
			int x2 = world->xStart + 1;
#endif
			mergeInto(world, x1, y, world->xStart, y);
			mergeInto(world, x2, y, world->xEnd, y);
		}
	}
}
