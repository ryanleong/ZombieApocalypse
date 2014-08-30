/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef AGENTS_H
#define AGENTS_H


typedef enum {
    MALE, FEMALE;
} gender_t;

typedef enum {
	CLEAN, INFECTED, DEAD
} status_t

typedef enum {
	NONE, AXE
} weapon_t

/**
 *  Humans have a gender, age,
 */
typedef struct {
    gender_t gender;
    int age;

    status_t status;
    weapon_t weapon;
    int rest;	// Keep track of humans lack of sleep


    // other properties might include velocity (x and y components), although
    // position is implicit in the 2D grid of the simulation world. Maybe a
    // boolean var to say whether the person has been exposed.

    human_t *next;
} human_t;

typedef struct {
    date_t time_of_infection;
    int decomposition_rate;
} zombie_t;


#endif // AGENTS_H

// vim: ts=4 sw=4 et
