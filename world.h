/**
 *  Defines a structure for each cell of the grid over which we are
 *  simulating a zombie outbreak.
 */

#ifndef WORLD_H
#define WORLD_H

// TODO: define how many rows/columns in the simulation area (NT).
#define GRID_ROWS 0
#define GRID_COLUMNS 0


typedef struct {
    // linked lists to represent all the humans and zombies in a particular 
    // square.
    human_t *humans;
    zombie_t *zombies;
} world_t;

world_t simulation_grid [GRID_ROWS] [GRID_COLUMNS];


#endif // WORLD_H

// vim: ts=4 sw=4 et
