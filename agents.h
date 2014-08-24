/**
 *  Defines structs to store relevant properties of humans and zombies.
 */

#ifndef AGENTS_H
#define AGENTS_H


typedef enum {
    MALE, FEMALE;
} gender_t;

/**
 *  Humans have a gender, age,
 */
typedef struct {
    gender_t gender;
    int age;

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
