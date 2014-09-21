#include "world.h"
#include "common.h"
#include "utils.h"
#include "random.h"

// local util functions
static Tile ** initialise_grid (World * world);
static void reset_tile (Tile * tile);


World * newWorld(unsigned int width, unsigned int height) {
    World * w;
    w = (World *) checked_malloc (sizeof(World));

    w->clock = 0;
    w->width = width;
    w->height = height;
#ifdef _OPENMP
	w->locks = (omp_lock_t *) checked_malloc(sizeof(omp_lock_t) * (width + 4));
#endif

    w->map = initialise_grid (w);

    return w;
}

/**
 *  Allocates memory for a new 2 dimensional matrix of Tiles, and sets
 *  the initial values of the tiles.
 */
    static Tile **
initialise_grid (World * world)
{
    Tile **grid = (Tile **) checked_malloc (sizeof (Tile *) * world->width);

    for (unsigned int x = 0; x < world->width; x ++)
    {
        grid [x] = (Tile *) checked_malloc (sizeof (Tile) * world->height);

        for (unsigned int y = 0; y < world->height; y ++)
            reset_tile (&(grid [x] [y]));

#ifdef _OPENMP
		omp_init_lock(world->locks + x);
#endif
    }

	return grid;
}

/**
 *  Check if the coordinates [row][column] are valid, ie. are they within
 *  the bounds of the matrix. Return value is true if the coordinates are
 *  valid, false if not.
 */
    bool
valid_coordinates (const World *world, int x, int y)
{
    // each index must be greater than or equal to 0, and strictly less
    // than the array size.
    if ((y < 0) || (y >= world->height))
        return false;

    if ((x < 0) || (x >= world->width))
        return false;

    return true;
}

void resetWorld(World * world) {
#ifdef _OPENMP
#pragma omp parallel for collapse(2) schedule(guided, 10)
#endif
    for (int row = 0; row < world->height; row ++)
    {
        for (int col = 0; col < world->width; col ++)
            reset_tile (&(world->map [row] [col]));
    }
}

    static void 
reset_tile (Tile * tile) 
{
    tile->entity_type = EMPTY;
}

void lockColumn(World * world, int x) {
#ifdef _OPENMP
	for (int i = x - 1; i <= x + 1; i++) {
		omp_set_lock(world->locks + i);
	}
#endif
}

void unlockColumn(World * world, int x) {
#ifdef _OPENMP
	for (int i = x - 1; i <= x + 1; i++) {
		omp_unset_lock(world->locks + i);
	}
#endif
}

/**
 *  Checks the adjacent tiles to see if any of them are unoccupied. If
 *  there is an unoccupied tile, its coordinates are stored in the
 *  variables pointed to by row and column, and this function will return
 *  a non zero value. Otherwise, this function returns 0
 *
 *  row and column initially store the coordinates of the tile to search
 *  around.
 */
    int
find_adjacent_space (const World *world, int *x, int *y)
{
    int i, j;

    for (i = *x - 1; i < *x + 1; i ++)
    {
        for (j = *y - 1; j < *y + 1; j ++)
        {
            // ignore coordinates that go outside the bounds of the matrix.
            if (!valid_coordinates (world, i, j))
                continue;

            // if the tile is unoccupied, choose it.
            if (world->map [i] [j].entity_type == EMPTY)
            {
                *x = i;
                *y = j;
                return 1;
            }
        }
    }

    return 0;
}

/** vim: set ts=4 sw=4 et : */
