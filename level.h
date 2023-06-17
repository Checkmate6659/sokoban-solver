#ifndef LEVEL_H
#define LEVEL_H

#include <inttypes.h>
#include <stdio.h>
#include "util.h"

//maximum level dimensions (i added a bit of extra clearance in case I want to do like sasquatch levels)
#define LEVEL_WIDTH 32 //max width = 30
#define LEVEL_HEIGHT 32 //max height = 19
#define MAX_BOXES ((LEVEL_WIDTH - 2) * (LEVEL_HEIGHT - 2) >> 1) //a reasonable number of boxes for a sokoban level
#define LEVEL_SIZE (LEVEL_WIDTH * LEVEL_HEIGHT)
#define COORDINATES(x, y) ((x) + (y) * LEVEL_WIDTH)
#define X_POSITION(i) ((i) % LEVEL_WIDTH) //WARNING: these did some weird stuff to me, so be careful!
#define Y_POSITION(i) ((i) / LEVEL_WIDTH)

//target square cannot be box, dead, wall; behind source cannot be box, wall
#define CAN_PUSH(tile, dir) (!(level[(tile) + (dir)] & WALL) && !(level[(tile) - (dir)] & BOX))

//the different tiles that can be on a Sokoban grid (defines must be used instead of enum, since enum can only have ints and not uint8s)
#define AIR 0
#define BOX 1
#define DEAD 2
#define WALL 3
#define GOAL 4
#define BOXONGOAL BOX | GOAL
typedef uint8_t Tile;

extern const int16_t DIRECTIONS[];

//the level stuff
extern Tile level[LEVEL_SIZE];
extern uint16_t boxes[MAX_BOXES], goals[MAX_BOXES];
extern uint16_t nboxes; //the number of boxes in the level
extern uint64_t boxes_hash;
extern uint8_t reachable_area[LEVEL_SIZE]; //the area the player can reach
extern uint16_t normalized_player_pos;
extern uint32_t depth;
extern uint64_t zobrist_keys[LEVEL_SIZE * 2];

void compute_reachable_area(uint16_t player_pos); //Compute the area reachable by the player (not including boxes)
void move(uint16_t box, int16_t dir);
void unmove(uint16_t box, int16_t dir);

#endif
