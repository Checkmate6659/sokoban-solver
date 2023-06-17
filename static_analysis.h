#ifndef STATIC_ANALYSIS_H
#define STATIC_ANALYSIS_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue>
#include <vector>
#include "level.h"
#include "util.h"

//same as CAN_PUSH, but boxes not taken into account
#define CAN_PUSH_STATIC(tile, dir) (level[(tile) + (dir)] != WALL && level[(tile) - (dir)] != WALL)
//same but takes dead squares into account
#define CAN_PUSH_STATIC_DEADSQ(tile, dir) (!(level[(tile) + (dir)] & DEAD) && level[(tile) - (dir)] != WALL)

void compute_dead_squares();
uint32_t compute_initial_lower_bound();

#endif
