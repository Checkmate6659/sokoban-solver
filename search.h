#ifndef SEARCH_H
#define SEARCH_H

#include <inttypes.h>
#include "level.h"
#include <stdio.h>
#include <stdlib.h>
#include <set> //TEMPORARY


#define PATH_NOT_FOUND (((uint32_t)-1) >> 1)
#define PATH_FOUND 123456789 //be careful when this is 0 (idk what happens then but sth weird)


uint32_t idastar_heuristic();
uint32_t idastar_search(uint32_t g, uint32_t bound);

#endif
