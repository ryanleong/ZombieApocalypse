#include "world.h"
#include "common.h"
#include "utils.h"

// local util functions
static Tile ** initialise_grid (unsigned int rows, unsigned int columns);
static void initTile (Tile *tile);
static void resetTile (Tile * tile);
static void destroyTile (Tile * tile);


World * newWorld(unsigned int width, unsigned int height) {
    World * w;
    w = (World *) checked_malloc (sizeof(World));

    w->clock = 0;
    w->width = width;
    w->height = height;
    w->map = initialise_grid (height, width);

    return w;
}

/**
 *  Allocates memory for a new 2 dimensional matrix of Tiles, and sets
 *  the initial values of the tiles.
 */
    static Tile **
initialise_grid (unsigned int rows, unsigned int columns)
{
    Tile **grid = (Tile **) checked_malloc (sizeof (Tile *) * rows);

    // Each row contains 'columns' columns.
    for (unsigned int row = 0; row < rows; row ++)
    {
        grid [row] = (Tile *) checked_malloc (sizeof (Tile) * columns);

        // initialise each tile in the row.
        for (unsigned int col = 0; col < columns; col ++)
            initTile (&(grid [row] [col]));
    }

    return grid;
}

/**
 *  Check if the coordinates [row][column] are valid, ie. are they within
 *  the bounds of the matrix. Return value is true if the coordinates are
 *  valid, false if not.
 */
    bool
valid_coordinates (World *world, int row, int column)
{
    // each index must be greater than or equal to 0, and strictly less
    // than the array size.
    if ((row < 0) || (row >= world->height))
        return false;

    if ((column < 0) || (column >= world->width))
        return false;

    return true;
}

void resetWorld(World * world) {
#ifdef _OPENMP
    int threads = omp_get_max_threads();
    int numThreads = MIN(MAX(world->width * world->height / 10, 1), threads);
    // each thread resets at least 10 elements
#pragma omp parallel for default(shared) num_threads(numThreads)
#endif
    for (int row = 0; row < world->height; row ++)
    {
        for (int col = 0; col < world->width; col ++)
            resetTile (&(world->map [row] [col]));
    }
}

static void initTile(Tile * tile) {
    resetTile(tile);
#ifdef _OPENMP
    omp_init_lock(&tile->lock);
#endif
}

static void resetTile(Tile * tile) {
    if (tile->entity != NULL) {
        disposeEntity(tile->entity);
        tile->entity = NULL;
    }
}

static void destroyTile(Tile * tile) {
    resetTile(tile);
#ifdef _OPENMP
    omp_destroy_lock(&tile->lock);
#endif
}

void lockTile(Tile * tile) {
#ifdef _OPENMP
    omp_set_lock(&tile->lock);
#endif
}

void unlockTile(Tile * tile) {
#ifdef _OPENMP
    omp_unset_lock(&tile->lock);
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
find_adjacent_space (World *world, int *row, int *column)
{
    int i, j;

    for (i = *row - 1; i < *row + 1; i ++)
    {
        for (j = *column - 1; j < *column + 1; j ++)
        {
            // ignore coordinates that go outside the bounds of the matrix.
            if (!valid_coordinates (world, i, j))
                continue;

            // if the tile is unoccupied, choose it.
            if (world->map [i] [j].entity == NULL)
            {
                *row = i;
                *column = j;
                return;
            }
        }
    }

    return 0;
}

/** vim: set ts=4 sw=4 et : */
