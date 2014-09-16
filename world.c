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

Tile * getFreeAdjacent(World * input, World * output, int x, int y) {
	Tile * t;
	if (GET_TILE_LEFT(input, x, y)->entity == NULL
			&& (t = GET_TILE_LEFT(output, x, y))->entity == NULL) {
		return t;
	} else if (GET_TILE_UP(input, x, y)->entity == NULL
			&& (t = GET_TILE_UP(output, x, y))->entity == NULL) {
		return t;
	} else if (GET_TILE_RIGHT(input, x, y)->entity == NULL && (t =
			GET_TILE_RIGHT(output, x, y))->entity == NULL) {
		return t;
	} else if (GET_TILE_DOWN(input, x, y)->entity == NULL
			&& (t = GET_TILE_DOWN(output, x, y))->entity == NULL) {
		return t;
	} else {
		return NULL;
	}
}
