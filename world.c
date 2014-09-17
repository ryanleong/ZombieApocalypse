#include "world.h"
#include "common.h"
#include "utils.h"

// local util functions
static tile_t ** initialise_grid (unsigned int rows, unsigned int columns);
static void init_tile (tile_t *tile);
static void reset_tile (tile_t * tile);


world_t * newWorld(unsigned int width, unsigned int height) {
    world_t * w;
    w = (world_t *) checked_malloc (sizeof(world_t));

    w->clock = 0;
    w->width = width;
    w->height = height;
    w->map = initialise_grid (height, width);

    return w;
}

/**
 *  Allocates memory for a new 2 dimensional matrix of tile_ts, and sets
 *  the initial values of the tiles.
 */
    static tile_t **
initialise_grid (unsigned int rows, unsigned int columns)
{
    tile_t **grid = (tile_t **) checked_malloc (sizeof (tile_t *) * rows);

    // Each row contains 'columns' columns.
    for (unsigned int row = 0; row < rows; row ++)
    {
        grid [row] = (tile_t *) checked_malloc (sizeof (tile_t) * columns);

        // initialise each tile in the row.
        for (unsigned int col = 0; col < columns; col ++)
            init_tile (&(grid [row] [col]));
    }

    return grid;
}

/**
 *  Check if the coordinates [row][column] are valid, ie. are they within
 *  the bounds of the matrix. Return value is true if the coordinates are
 *  valid, false if not.
 */
    bool
valid_coordinates (const world_t *world, int row, int column)
{
    // each index must be greater than or equal to 0, and strictly less
    // than the array size.
    if ((row < 0) || (row >= world->height))
        return false;

    if ((column < 0) || (column >= world->width))
        return false;

    return true;
}

void resetWorld(world_t * world) {
#ifdef _OPENMP
    int threads = omp_get_max_threads();
    int numThreads = MIN(MAX(world->width * world->height / 10, 1), threads);
    // each thread resets at least 10 elements
#pragma omp parallel for default(shared) num_threads(numThreads)
#endif
    for (int row = 0; row < world->height; row ++)
    {
        for (int col = 0; col < world->width; col ++)
            reset_tile (&(world->map [row] [col]));
    }
}

/**
 *  Initialises a tile_t struct to be empty.
 */
    static void
init_tile (tile_t *tile)
{
    reset_tile (tile);
#ifdef _OPENMP
    omp_init_lock(&tile->lock);
#endif
}

    static void 
reset_tile (tile_t * tile) 
{
    tile->entity_type = EMPTY;
}

void lockTile(tile_t * tile) {
#ifdef _OPENMP
    omp_set_lock(&tile->lock);
#endif
}

void unlockTile(tile_t * tile) {
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
find_adjacent_space (const world_t *world, int *row, int *column)
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
            if (world->map [i] [j].entity_type == EMPTY)
            {
                *row = i;
                *column = j;
                return 1;
            }
        }
    }

    return 0;
}

/** vim: set ts=4 sw=4 et : */
