#ifndef STATIC_ANALYSIS_H
#define STATIC_ANALYSIS_H
#include <stdlib.h>
#include <string.h>
#include "level.h"

//same as CAN_PUSH, but boxes not taken into account
#define CAN_PUSH_STATIC(tile, dir) ((level[(tile) + (dir)] != WALL && level[(tile) - (dir)] != WALL))

void compute_dead_squares();

#endif
